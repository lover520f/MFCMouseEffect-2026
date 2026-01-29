# Text Effect Motion Regression

## Summary
- After switching to DirectWrite/Direct2D for emoji rendering, the floating text drift path felt different from the original GDI+ behavior.

## Fix
- Restore the original motion math by applying the drift offsets (`xPos`, `yOffset`) both:
  - in the rotation pivot, and
  - in the text layout origin.

## Result
- The floating text path and feel now match the pre-change behavior.
