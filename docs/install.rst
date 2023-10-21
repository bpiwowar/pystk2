Installation
============

Using pip
---------

.. code-block:: bash

   pip install pystk2

From source
-----------

.. code-block:: bash

   pip install .

Development
-----------

Clone the repository `https://github.com/bpiwowar/pystk2 <https://github.com/bpiwowar/pystk2>`_.
For easier development, it is recommended to install pystk2 directly through ``cmake``.

.. code-block:: bash

   pip install -e .

CMake will place a copy of the library in the top level directly, with allows any examples to run from that directory.

Documentation
-------------

To build local documentation

.. code-block:: bash

   cd docs
   make html
