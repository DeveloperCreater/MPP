## M++ Icon Assets

- `mpp_file_icon.svg`: vector icon you can use in docs, websites, and as a source for a Windows `.ico`.

### Make a Windows `.ico` (optional)

If you install ImageMagick, you can convert the SVG to an `.ico`:

```powershell
magick assets\mpp_file_icon.svg -background none -define icon:auto-resize=256,128,64,48,32,16 assets\mpp_file_icon.ico
```

