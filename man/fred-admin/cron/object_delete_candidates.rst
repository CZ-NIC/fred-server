
Deleting objects
----------------

Delete registered objects waiting for deletion (\ *delete candidates*).

This task is also a part of :option:`--object_regular_procedure`.

If run separately, it should be preceded by :option:`--object_update_states`.

Cron recommendations
^^^^^^^^^^^^^^^^^^^^

Frequency: same as :option:`--object_update_states`

Synopsis
^^^^^^^^

``fred-admin --object_delete_candidates --object_delete_types=<list> [--object_delete_debug] [--object_delete_limit=<integer>] [--object_delete_parts=<integer>] [--object_delete_spread_during_time=<integer>]``

Options
^^^^^^^^

.. option:: --object_delete_candidates

   The command switch.

.. option:: --object_delete_debug

   Enable additional output.

   Default: off

.. option:: --object_delete_limit=<integer>

   Limit the number of objects deleted in one run.

   Default: no limit

.. option:: --object_delete_parts=<integer>

   Limit deletion to 1/n fraction of objects' total.

   Default: 1 (all)

.. option:: --object_delete_spread_during_time=<integer>

   Stretch deletion over the time specified in **seconds**
   (0 = delete everything immediately).

   Default: 0 [seconds]

.. option:: --object_delete_types=<list of types>

   Delete only these types of registrable objects, **mandatory option**.

   Types: contact, domain, keyset, nsset

   **If this option is omitted, nothing will be deleted.**

   Default: none


Example
^^^^^^^

``fred-admin --object_delete_candidates --object_delete_types contact,keyset,nsset``

   Immediately delete all non-domain objects that are waiting for deletion.

``fred-admin --object_delete_candidates --object_delete_types domain --object_delete_parts 10 --object_delete_spread_during_time 120``

   Delete 1/10 of domains that are waiting for deletion,
   stretch it over 2 minutes.
