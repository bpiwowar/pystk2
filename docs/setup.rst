.. _graphics:

Graphics Setup
==============

Before you can use pystk2 you need to setup the OpenGL rendering engine and graphics settings.
There are three default settings ``GraphicsConfig::ld`` (lowest),  ``GraphicsConfig::sd`` (medium),  ``GraphicsConfig::hd`` (high).
Depending on your graphics hardware each setting might perform slightly differently (``ld`` fastest, ``hd`` slowest).
To setup pystk2 call:

.. code-block:: python

    pystk2.init(pystk2.GraphicsConfig.hd())
    # Only call init once per process
    ... # use pystk2
    pystk2.clean() # Optional, will be called atexit
    # Do not call pystk2 after clean

Headless rendering
------------------

If you want GPU rendering (to access ``render_data``) but don't need the on-screen window to update,
set ``display=False`` on the graphics config:

.. code-block:: python

    config = pystk2.GraphicsConfig.hd()
    config.display = False
    pystk2.init(config)

This initializes the full OpenGL context and render targets, so ``race.render_data`` will
contain color, depth, and instance segmentation buffers, but the display window is not
refreshed each step. This is useful for RL training where you only need pixel observations
without visual output.

To fully disable rendering (no OpenGL context at all), use ``GraphicsConfig.none()``.

.. include:: auto/graphicsconfig.grst
