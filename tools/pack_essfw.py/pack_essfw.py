#!/usr/bin/env python3
"""Pack a Cortex-M .bin into an 8 KiB slot header.

Adapted from Arm China host_manifest_tool (JSON -k/-m/-o), not a port of
the C+OpenSSL tree. Device layout is the 1-image manifest + app_metadata_t.

  manifest_header (24)
  img_header (24)
  SHA-256 of plaintext (32)
  cipher_info (36)  — zeros on S32K312
  app_metadata_t (132)
  uncompressed P-256 pubkey (65)
  ECDSA r||s (64)
  0xFF pad to 8192

  python3 pack_essfw.py --bin app.bin --slot a --key lab.pem -o slot_a.essfw
  python3 pack_essfw.py -k key_desc.json -m mnft_desc.json -o slot_a.essfw
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import struct
from pathlib import Path

from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec, utils

MAGIC = 0xA1BC2FD8
META_MAGIC = 0xAABBCCDD
LAYOUT_VER = 1
HEADER_SIZE = 8192
F195 = re.compile(r"^[0-9]{2}\.[0-9]{2}\.[0-9]{2}$")
FLAGS = 1 | (3 << 3) | (6 << 8) | (3 << 16)
SIGNED_SIZE = 313
TOTAL = 377
ALLOWED_SIG = "ECDSA_256_SHA256"
ALLOWED_HASH = "SHA256"

SLOT = {
    "a": (0, 0x00442000, 0x000C8000),
    "b": (1, 0x0050C000, 0x000C8000),
}


def _pad(s: str, n: int) -> bytes:
    b = s.encode("ascii", "strict")
    if len(b) >= n:
        raise SystemExit(f"field longer than {n - 1}: {s!r}")
    return b + b"\x00" * (n - len(b))


def uncompressed_pub(pub) -> bytes:
    n = pub.public_numbers()
    return b"\x04" + n.x.to_bytes(32, "big") + n.y.to_bytes(32, "big")


def _u32(v) -> int:
    if isinstance(v, int):
        return v
    s = str(v).strip()
    return int(s, 0)


def _load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def apply_descriptors(args: argparse.Namespace) -> None:
    """Fill CLI fields from Arm-style key_desc.json + mnft_desc.json."""
    key_path = Path(args.k)
    mnft_path = Path(args.m)
    key = _load_json(key_path)
    mnft = _load_json(mnft_path)

    sig = key.get("mnft_sig_cfg") or {}
    dgst = key.get("img_dgst_cfg") or {}
    enc = key.get("img_enc_cfg")
    sch = sig.get("mnft_sig_sch")
    hsh = dgst.get("img_hash_sch")
    puk_h = sig.get("pubkey_hash_sch", ALLOWED_HASH)
    if sch != ALLOWED_SIG:
        raise SystemExit(f"mnft_sig_sch must be {ALLOWED_SIG}, not {sch!r}")
    if hsh != ALLOWED_HASH:
        raise SystemExit(f"img_hash_sch must be {ALLOWED_HASH}, not {hsh!r}")
    if puk_h != ALLOWED_HASH:
        raise SystemExit(f"pubkey_hash_sch must be {ALLOWED_HASH}, not {puk_h!r}")
    if enc:
        raise SystemExit("img_enc_cfg is not used on S32K312 (SB-D12); omit it")

    prv = sig.get("mnft_prvkey")
    if not prv:
        raise SystemExit("mnft_sig_cfg.mnft_prvkey missing")
    args.key = str((key_path.parent / prv).resolve()) if not Path(prv).is_absolute() else prv

    imgs = mnft.get("imgs") or []
    if len(imgs) != 1:
        raise SystemExit("imgs must have exactly 1 entry (img_num=1)")
    if mnft.get("ext_bin"):
        raise SystemExit("ext_bin is not used")
    img = imgs[0]
    if img.get("is_enc"):
        raise SystemExit("is_enc must be false on S32K312")
    args.bin = str((mnft_path.parent / img["path"]).resolve()) if not Path(img["path"]).is_absolute() else img["path"]

    static = _u32(img["static_addr"])
    load = _u32(img["load_addr"])
    entry = _u32(img["entry"])
    if static != load or load != entry:
        raise SystemExit("S32K312 XIP requires static_addr == load_addr == entry")

    slot = img.get("slot")
    if slot is None:
        for name, (_sid, addr, _mx) in SLOT.items():
            if addr == static:
                slot = name
                break
    if slot not in SLOT:
        raise SystemExit(f"unknown slot {slot!r} / static_addr 0x{static:08X}")
    expect = SLOT[slot][1]
    if static != expect:
        raise SystemExit(f"slot {slot} static_addr must be 0x{expect:08X}")
    args.slot = slot

    ident = mnft.get("app_identity") or {}
    args.sw_version = ident.get("sw_version", args.sw_version)
    args.app_name = ident.get("app_name", args.app_name)
    args.product_family = ident.get("product_family", args.product_family)
    args.part_number = ident.get("part_number", args.part_number)
    args.hw_version = ident.get("hw_version", args.hw_version)
    args.rxswin = ident.get("rxswin", args.rxswin)
    args.anti_rollback = int(ident.get("anti_rollback", args.anti_rollback))


def pack(args: argparse.Namespace) -> int:
    if not F195.match(args.sw_version):
        raise SystemExit("F195 must be MM.mm.pp (e.g. 00.01.00), no leading v")

    image = Path(args.bin).read_bytes()
    slot_id, img_addr, img_max = SLOT[args.slot]
    if len(image) == 0 or len(image) > img_max:
        raise SystemExit(f"image size {len(image)} out of range (1..{img_max})")

    priv = serialization.load_pem_private_key(Path(args.key).read_bytes(), password=None)
    if not isinstance(priv, ec.EllipticCurvePrivateKey) or priv.curve.name != "secp256r1":
        raise SystemExit("key must be ECDSA P-256 (secp256r1)")
    pub_bytes = uncompressed_pub(priv.public_key())

    digest = hashlib.sha256(image).digest()
    meta = struct.pack(
        "<I16s12sIIII16s16s16sI32s",
        META_MAGIC,
        _pad(args.app_name, 16),
        _pad(args.sw_version, 12),
        0,
        img_addr,
        len(image),
        0,
        _pad(args.product_family, 16),
        _pad(args.part_number, 16),
        _pad(args.hw_version, 16),
        args.anti_rollback,
        _pad(args.rxswin, 32),
    )
    cipher = struct.pack("<I16s16s", 0, b"\x00" * 16, b"\x00" * 16)
    unsigned = struct.pack(
        "<IIIIIIIIIIII32s",
        MAGIC,
        LAYOUT_VER,
        1,
        TOTAL,
        FLAGS,
        1,
        0,
        args.anti_rollback,
        img_addr,
        img_addr,
        len(image),
        img_addr,
        digest,
    ) + cipher + meta + pub_bytes
    if len(unsigned) != SIGNED_SIZE:
        raise SystemExit(f"unsigned size {len(unsigned)} != {SIGNED_SIZE}")

    sig = priv.sign(unsigned, ec.ECDSA(hashes.SHA256()))
    r, s = utils.decode_dss_signature(sig)
    raw = r.to_bytes(32, "big") + s.to_bytes(32, "big")
    blob = unsigned + raw
    if len(blob) != TOTAL:
        raise SystemExit(f"blob {len(blob)} != {TOTAL}")
    Path(args.o).write_bytes(blob + b"\xff" * (HEADER_SIZE - len(blob)))
    print(
        f"wrote {args.o}  mnfst=0x{MAGIC:08X}  slot={slot_id}  "
        f"img={len(image)}  load=0x{img_addr:08X}  F195={args.sw_version}  enc=off"
    )
    return 0


def main() -> int:
    p = argparse.ArgumentParser(
        description="Pack ESSI slot header (Arm 1-image manifest + app_metadata)"
    )
    p.add_argument("-k", help="key_desc.json (Arm host_manifest_tool)")
    p.add_argument("-m", help="mnft_desc.json (Arm host_manifest_tool)")
    p.add_argument("--bin", help="plaintext App .bin (CLI mode)")
    p.add_argument("--slot", choices=("a", "b"), default="a")
    p.add_argument("--key", help="PEM ECDSA P-256 private key (CLI mode)")
    p.add_argument("--sw-version", default="00.01.00", help="F195 MM.mm.pp")
    p.add_argument("--app-name", default="FreeRTOSApp")
    p.add_argument("--product-family", default="S32K312")
    p.add_argument("--part-number", default="S32K312-EVB")
    p.add_argument("--hw-version", default="REV-A")
    p.add_argument("--rxswin", default="ESS-S32K-000001")
    p.add_argument("--anti-rollback", type=int, default=1)
    p.add_argument("-o", required=True)
    args = p.parse_args()

    if args.k or args.m:
        if not args.k or not args.m:
            raise SystemExit("JSON mode needs both -k and -m")
        apply_descriptors(args)
    elif not args.bin or not args.key:
        raise SystemExit("CLI mode needs --bin and --key (or use -k/-m JSON)")

    return pack(args)


if __name__ == "__main__":
    raise SystemExit(main())
