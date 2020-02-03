
Administering registrable objects
---------------------------------

Perform regular administration of registrable objects. The procedure comprises
the following tasks:

* Update object states according to their life cycles.
* Notify contacts (by email) and registrars (by EPP polling)
  about critical state changes, such as expiration of domains.
* Notify registrars if their credit is too low.
* Delete objects waiting for deletion (*delete candidates*).

This command is a shortcut for the following tasks run in this order:

* 2x :option:`--object_update_states` CRITICAL,
* :option:`--notify_state_changes`,
* :option:`--poll_create_statechanges`,
* :option:`--object_delete_candidates` CRITICAL,
* poll_create_low_credit (no separate command),
* notify_letters_create (no separate command)

.. NOTE wrong cmd name in help (`update_object_states`)

Cron recommendations
^^^^^^^^^^^^^^^^^^^^

Frequency: twice a day (e.g. 00:00 and 12:00)

.. Important:: It is critical for operation of the Registry to set up this task
   or an equivalent set of separate critical tasks as listed above!

Synopsis
^^^^^^^^

``fred-admin --object_regular_procedure --object_delete_types=<list> [--object_delete_debug] [--notify_except_types=<list>] [--object_delete_limit=<integer>] [--poll_except_types=<list>]``

Options
^^^^^^^^

.. option:: --object_regular_procedure

   The command switch.

.. option:: --notify_except_types=<list of types>

   See the command :option:`--notify_state_changes`.

.. option:: --object_delete_types, --object_delete_debug, --object_delete_limit

   See the command :option:`--object_delete_candidates`.

.. option:: --poll_except_types=<list of types>

   See the command :option:`--poll_create_statechanges`.

Example
^^^^^^^^

``fred-admin --object_regular_procedure --object_delete_types contact,keyset,nsset --notify_except_types validationWarning2,notValidated --poll_except_types validationWarning1,notValidated``
