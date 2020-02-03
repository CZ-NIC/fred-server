
Updating object states
----------------------

Update state flags of registrable objects according to their life cycle.

This task is also part of :option:`--object_regular_procedure`.

.. Important:: Run it twice, if you intend to follow it with
   :option:`--object_delete_candidates`.

Cron recommendations
^^^^^^^^^^^^^^^^^^^^

Frequency: twice a day (e.g. 00:00 and 12:00)

.. Important:: This task is critical for operation of the Registry and it must
   be set up standalone or as part of the :option:`--object_regular_procedure`.

Synopsis
^^^^^^^^
``fred-admin --object_update_states [--object_id=<integer>]``

Options
^^^^^^^^

.. option:: --object_update_states

   The command switch.

.. option:: --object_id=<integer>

   Update only a single registered object specified with its numeric
   identifier (the ``id`` attribute e.g. from the ``domain`` table).

Example
^^^^^^^^

``fred-admin --object_update_states --object_id 343741``

See also
^^^^^^^^

See also the concept `Life cycle of registrable objects
<https://fred.nic.cz/documentation/html/Concepts/LifeCycle>`_
in the documentation.
