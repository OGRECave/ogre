# Sample Thumbnails

- The resolution is `128x128px`, which is a power of 2 (POT).
- However, the images are displayed at `128x96px`, which corresponds to an aspect of 4:3, but is not POT.
- Therefore, you should crop the image at `171x128px` and resize accordingly