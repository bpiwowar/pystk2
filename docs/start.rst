Quick start
===========

Let's walk through a simple example on how to use pystk2.
You'll first need to setup the rendering engine.
SuperTuxKart uses a lot of global memory objects, some of them should only be initilized once.
Hence, you should only setup the rendering engine *once* per process.

.. code-block:: python

    config = pystk2.GraphicsConfig.hd()
    config.screen_width = 800
    config.screen_height = 600
    pystk2.init(config)

This setup uses the high-definition graphics preset and sets the resolution to 800 x 600.

Now we're ready to start the race.
You may play as many races as you want, but you can only run *one* race per process.
If you try to start (or setup) a second race before completing the first, the wrapper will raise an exception and eventually crash.

.. code-block:: python

    config = pystk2.RaceConfig()
    config.num_kart = 2 # Total number of karts
    config.players[0].controller = pystk2.PlayerConfig.Controller.AI_CONTROL

    config.track = 'lighthouse'
    
    race = pystk2.Race(config)

This race configuration plays the ``lighthouse`` track with a total of 2 karts, one of which is player controlled.
By default there is only one player ``len(config.players)==1`` and all other karts are non-controllable AI karts.

Next, let's start the race and play for a 100 steps.

.. code-block:: python

    race.start()
    for n in range(100):
        race_ended = race.step()

After each step, you can access rendered images through ``race.render_data``.
Each entry corresponds to one camera and provides color, depth, and instance segmentation buffers:

.. code-block:: python

    for data in race.render_data:
        color = data.image       # uint8 array (H x W x 3)
        depth = data.depth       # float array (H x W)
        labels = data.instance   # uint32 array (H x W)

See :ref:`race` for a full documentation of the race object and :ref:`data` for the render data format.

Finally, delete the current race object before exiting or starting a new race.

.. code-block:: python

    race.stop()
    del race
