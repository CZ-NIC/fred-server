
Notifying about object events
-----------------------------

Send emails notifying about critical events in the life cycle of registrable
objects.

This task is also part of :option:`--object_regular_procedure`.

If run separately, it should be preceded by :option:`--object_update_states`.

Cron recommendations
^^^^^^^^^^^^^^^^^^^^

Frequency: optionally after :option:`--object_update_states`

Synopsis
^^^^^^^^

``fred-admin --notify_state_changes [--notify_debug] [--notify_except_types=<list>] [--notify_limit=<integer>]``

Options
^^^^^^^^

.. option:: --notify_state_changes

   The command switch.

.. option:: --notify_debug

   Enable additional output.

   Default: off

.. option:: --notify_except_types=<list of states>

   List of object states ignored in notification.

   States that are usually notified by email are: deleteCandidate, expired,
   notValidated, outzoneUnguarded, outzoneUnguardedWarning, validationWarning2.

   Default: none (empty list)

.. option:: --notify_limit=<integer>

   Limit the number of emails generated in one pass (0 = no limit).

   Default: no limit

Example
^^^^^^^^

``fred-admin --notify_state_changes --notify_except_types validationWarning2,notValidated --notify_limit 10``

See also
^^^^^^^^

See also the concept `Communication
<https://fred.nic.cz/documentation/html/Concepts/Communication.html>`_
in the documentation.
