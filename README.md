# ue4-simple-panoramic-exporter

Simple actor that exports panoramas based off of Unreal's internal cubemap system

This is an open source release of the Simple Panoramic Exporter located on the Unreal Engine Marketplace.

https://www.unrealengine.com/marketplace/en-US/product/simple-panoramic-exporter

# Overview

Exports image sequences in BMP, PNG, JPG, EXR, HDR formats into your project's Saved\Panoramas folder.

It only exports image sequences and does not do any assembly of said image sequences, it is up to you to assemble these image sequences.

# Demo

Example Renderings:

- [8k x 4k Meadow Marketplace Test](https://www.youtube.com/watch?v=r5FPVvUPLSo)
- [HDR 4k x 2k Basketball Marketplace Test](https://www.youtube.com/watch?v=Xnd72X_70hI)

If you make anything with this tool, please submit a pull request if you want your thing added to this thing.

# Support

This is no longer supported officially by its author. This is released under MIT with zero plans for future updates. If you would like to discuss the Simple Panoramic Exporter, one place to do so might be the [Gamemakin LLC Discord Server](http://discord.gamemak.in/), the original home of this plugin.

# Installation

If you go to the Releases tab, binaries have been built for the launcher version of Unreal Engine. They should not need to be recompiled unless for some reason you're already compiling things. You will need to compile these binaries again if you are using a source build of the engine, which isn't a problem because you're already using a source build of the engine.

Extract Plugins\PanoramicExporter into your Project's Plugins folder. You can also install it as an Engine plugin.

# Usage

Once installed, you can place a Panoramic Exporter into your level and configure it via the details panel. When an exporter is in an active play session it will immediately start exporting. You can animate this actor using Sequencer and it'll export a 360 capture wherever it is located.

Rotation is not supported. It just isn't. It is because I'm using some unwrap logic that Unreal already provides and it doesn't support rotation.

Screen space effects are not supported, because this works using Epic's cube map capture implementation and it doesn't support screen space effects. It is also really hard to do screen space effects when there is no screen and you are doing a 360 capture.

This only does monoscopic, it does not support stereoscopic. If you put two monoscopic exporters side by side, what you end up with is something that works if you're looking straight ahead but will make you cross-eyed when you look behind you. You can't just add a second camera to simulate a second eyeball.

# Original Marketplace Tutorial / Trailer

[https://www.youtube.com/watch?v=lrxKo8lzSAE](https://www.youtube.com/watch?v=lrxKo8lzSAE)
