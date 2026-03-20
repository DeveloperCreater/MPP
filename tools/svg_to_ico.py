from __future__ import annotations

from pathlib import Path
from io import BytesIO

from PIL import Image
from svglib.svglib import svg2rlg
from reportlab import rl_config

# Must set backend BEFORE importing renderPM.
# Use rlPyCairo (works on Windows with pycairo).
rl_config.renderPMBackend = "rlPyCairo"

from reportlab.graphics import renderPM


def main() -> None:
    root = Path(__file__).resolve().parents[1]
    # Use the flat SVG for conversion (no gradients/filters).
    svg_path = root / "assets" / "mpp_file_icon_flat.svg"
    ico_path = root / "assets" / "mpp_file_icon.ico"

    if not svg_path.exists():
        raise SystemExit(f"Missing SVG: {svg_path}")

    # Render SVG to a high-res raster.
    drawing = svg2rlg(str(svg_path))
    png_bytes = renderPM.drawToString(drawing, fmt="PNG", dpi=192)  # ~512px for 512 viewBox
    base = Image.open(BytesIO(png_bytes)).convert("RGBA")

    sizes = [16, 24, 32, 48, 64, 128, 256]
    images = [base.resize((s, s), Image.Resampling.LANCZOS) for s in sizes]

    ico_path.parent.mkdir(parents=True, exist_ok=True)
    images[0].save(ico_path, format="ICO", sizes=[(s, s) for s in sizes])
    print(f"Wrote {ico_path}")


if __name__ == "__main__":
    main()
