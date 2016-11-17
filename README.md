# 2dVolumetricLighting

## Background
Simplified demonstration of 2d image based shadowing and 2d planar lights.  This demo uses a fixed camera.

I originally explored these approaches in a console game.  This is a separate implementation: it's largely similar but some of the rough edges I haven't bothered polishing.

![Point light, debug ui][screenshot0]

## Lighting

Lighting is calculating in the xy plane at a fixed z distance.  They depend on "normal" maps which indicate directionality - I used [SdfNormal](https://github.com/jvalenzu/SdfNormal) to generate the associated _OUTPUT data here.  This is done in the Planar set of shaders.

There is a prepass which renders out an OBB per light (slightly rotated into screen) and which ORs in a unique bit per light to allow the Planar shader to only calculate attenuation for relevant pixels.  It aims to be conservative.

Range and attenuation are, admittedly, not physically modeled.

![Output from test0][output0]

Both this demo and the original code suffer from a singularity when the light source passes in front of an object using planar lighting.  It's meant to highlight the outlines of geometry, and as such the approach works best with collidable geometry you can't get in front of.  In the original code, normal maps were encoded with a separate parameter to interpolate between planar and non-planar lighting when passing in front of geometry.

## Shadows
Shadows are 2d volumetric, simulating a participating medium.

Some 2d approaches use physics raycasts to generate shadow volumes.  For each light source (lsp), physics raycasts are sent radially, the intersection point (if there is one) defining a vertex on leading edge.  Geometry is extruded and the in-world geometry is lightened or darkened depending on desired effect.  Variants of this approach could include doing a full-screen pass including the geometry instead of rendering in world.  This approach works well provided that (a) the collision geometry is a good approximation to the render data, (b) the resolution of raycast is sufficiently fine to pick out details of the collision geometry, (c) your scene construction is amenable to planar physics tests.

Other approaches, like this one, are image based.  Shadow casters are rendered to an offscreen texture.  For each lsp, an additional pass performs a raymarch from the lsp to each pixel location to determine whether it's occluded.

There are quite a few image based implementations ([mattdesc1](https://github.com/mattdesl/lwjgl-basics/wiki/2D-Pixel-Perfect-Shadows) and [catalinzima](http://www.catalinzima.com/2010/07/my-technique-for-the-shader-based-dynamic-2d-shadows/) to pick two popular ones links).  These implementations render the shadow casters with the lsp at the origin.  A second pass is performed (1d raymarching) into a 1d render texture.  This pass operates on polar coordinates to determine the minimum distance at which a "collision" occurs.  Use of the 1d render texture for lookups first require determining the distance from a screen position to the lsp and then comparing against the stored 1d lookup.  This technique works well provided (a) the lsp is onscreen or very close to being onscreen and (b) there is not great z-depth variance in the shadow casters and/or lsp.

This approach is similar to mattdesc1.  One difference is we don't require the lsp to be the center of the 1d raymarch scene.  This is because we take a more expensive approach to map the 1d raymarch texture to our normalized t (specifically, line clipping to the border - border in shader.h).  We render shadow casters, then for each lsp generate a 1d raymarch texture, and a final pass to combine results.  In this demo, I do one 1d->2d "final" pass per shadowing light.

## Building

Tested on Mac OSX, Windows 10.

### External Libraries
* [GLFW](https://github.com/glfw/glfw)           - https://github.com/glfw/glfw
* [GL](http://khronos.org)                       - http://khronos.org
* [Dear imgui](https://github.com/ocornut/imgui) - https://github.com/ocornut/imgui

## Assets
Most sprite data (Beam.png, TreeApple.png) from the [Small World sprite set](http://www.lostgarden.com/2009/03/dancs-miraculously-flexible-game.html).  The associated normal data (TreeApple_OUTPUT.png) built from the sprite data.

Avatar.png from [LostGarden](http://www.lostgarden.com/2005/03/download-complete-set-of-sweet-8-bit.html)

[output0]: Etc/ExampleOutput.png
[screenshot0]: Etc/Screenshot0.png
