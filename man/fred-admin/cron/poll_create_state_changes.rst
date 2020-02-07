
Poll-messaging about object events
------------------------------------------

Prepare notifications in the EPP polling about critical events in the life
cycle of registrable objects.

This task is also part of :option:`--object_regular_procedure`.

If run separately, it should be preceded by :option:`--object_update_states`.

Cron recommendations
^^^^^^^^^^^^^^^^^^^^

Frequency: optionally after :option:`--object_update_states`

Synopsis
^^^^^^^^

``fred-admin --poll_create_statechanges [--poll_debug] [--poll_except_types=<list>] [--poll_limit=<integer>]``

Options
^^^^^^^^

.. option:: --poll_create_statechanges

   The command switch.

.. option:: --poll_debug

   Enable additional output.

   Default: off

.. option:: --poll_except_types=<list of states>

   List of object states ignored in notification.

   States that are usually notified by polling are: deleteCandidate,
   expirationWarning, expired, notValidated, outzoneUnguarded,
   validationWarning1.

   Default: none (empty list)

.. option:: --poll_limit=<integer>

   Limit the number of poll messages generated in one run (0 = no limit).

   Default: no limit


Example
^^^^^^^

``fred-admin --poll_create_statechanges --poll_except_types validationWarning1,notValidated --poll_limit 50``

See also
^^^^^^^^

See also the concept `Communication
<https://fred.nic.cz/documentation/html/Concepts/Communication.html>`_
in the documentation.
