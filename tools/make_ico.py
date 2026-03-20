from __future__ import annotations

from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


def render(size: int) -> Image.Image:
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Background
    bg = (15, 23, 42, 255)  # slate-900
    accent = (14, 165, 233, 255)  # sky-500
    d.rounded_rectangle((0, 0, size - 1, size - 1), radius=max(8, size // 10), fill=bg)
    d.rounded_rectangle((3, 3, size - 4, size - 4), radius=max(8, size // 10), outline=accent, width=max(2, size // 32))

    # Text
    text = "M++"
    # Try common Windows fonts; fall back to default.
    font = None
    for name in ("segoeuib.ttf", "seguisb.ttf", "arialbd.ttf", "arial.ttf"):
        try:
            font = ImageFont.truetype(name, size=int(size * 0.42))
            break
        except Exception:
            continue
    if font is None:
        font = ImageFont.load_default()

    tw, th = d.textbbox((0, 0), text, font=font)[2:]
    x = (size - tw) // 2
    y = int(size * 0.34) - th // 2

    # Subtle shadow then white text
    d.text((x + 2, y + 3), text, font=font, fill=(0, 0, 0, 120))
    d.text((x, y), text, font=font, fill=(255, 255, 255, 255))

    # Accent underline
    uy = int(size * 0.72)
    uw = int(size * 0.48)
    ux = (size - uw) // 2
    d.rounded_rectangle((ux, uy, ux + uw, uy + max(3, size // 32)), radius=max(2, size // 64), fill=accent)

    return img


def main() -> None:
    root = Path(__file__).resolve().parents[1]
    out = root / "assets" / "mpp_file_icon.ico"
    out.parent.mkdir(parents=True, exist_ok=True)

    # ICO should contain multiple sizes
    sizes = [16, 24, 32, 48, 64, 128, 256]
    images = [render(s) for s in sizes]
    images[0].save(out, format="ICO", sizes=[(s, s) for s in sizes])
    print(f"Wrote {out}")


if __name__ == "__main__":
    main()

