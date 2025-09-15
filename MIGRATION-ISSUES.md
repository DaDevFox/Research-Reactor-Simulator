# Migration Issues: SVG/Image Rendering with NanoVG/NanoGUI

## Symptom
- After migrating to a newer version of NanoGUI (and possibly NanoVG), components that previously rendered SVGs or images now appear as solid block colors (commonly blue or black).
- This affects widgets such as `CustomImagePanel` and `CustomImageView`, which use `nvgImagePattern` and `nvgImageSize` for rendering.

## Likely Causes
1. **Image Handle Invalid or Not Loaded**
   - If `nvgCreateImage*` fails (e.g., due to missing file, unsupported format, or OpenGL context issues), it returns 0 or -1. Passing this to `nvgImagePattern` results in a solid color fill.
2. **SVG Support Not Enabled or Broken**
   - NanoVG requires explicit SVG support (and sometimes additional libraries). If SVG support is not compiled in, or the loader fails, the image will not render.
3. **OpenGL Context or NanoVG Flags**
   - NanoVG must be initialized with the correct flags (e.g., `NVG_IMAGE_PREMULTIPLIED`, `NVG_ANTIALIAS`, etc.). Missing or incorrect flags can break image rendering.
4. **API or Behavior Changes in NanoVG/NanoGUI**
   - Newer versions may have changed image loading APIs, required flags, or stricter error handling.
5. **Resource Path or File Issues**
   - If the migration changed resource locations, images may not be found at runtime.

## Diagnostic Steps
- Check return values of all `nvgCreateImage*` calls. Log or assert if the handle is <= 0.
- Confirm that SVG support is enabled in your NanoVG build (and that any required dependencies are present).
- Ensure that the OpenGL context is valid and active when loading images.
- Double-check resource paths and working directory after migration.
- Review NanoVG/NanoGUI release notes for breaking changes in image/SVG handling.

## Solutions
- Add error checking after image loads; display a warning if the handle is invalid.
- If using SVGs, ensure NanoVG is built with SVG support and all dependencies are present.
- If using vcpkg or CMake, verify that the correct NanoVG/NanoGUI build options are set.
- Consider updating image loading code to match new APIs if needed.
- If the problem persists, try loading a simple PNG/JPG to isolate SVG-specific issues.

## References
- [NanoVG Issues: Solid Color Instead of Image](https://github.com/memononen/nanovg/issues)
- [NanoGUI Issues: Image/SVG Rendering](https://github.com/wjakob/nanogui/issues)
- [NanoVG Documentation](https://github.com/memononen/nanovg)

## Next Steps
- Audit all image loading code for error handling.
- Test with known-good image files and log results.
- If SVGs are critical, ensure your NanoVG build supports them and dependencies are present.
- Update this document as new issues or solutions are discovered during migration.
