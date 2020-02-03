
Description
-----------

The program ``fred-admin`` is a command-line tool for FRED administration.

There are two basic types of commands: init tasks and cron tasks.

Init tasks
^^^^^^^^^^^^^

**Init tasks** must be run to initialize a new Registry. The following tasks
are the necessary minimum and they must be executed in this order:

* Creating a zone: ``--zone_add``
* Adding zone nameservers: ``--zone_ns_add``
* Creating a registrar: ``--registrar_add``
* Granting access to a zone: ``--registrar_add_zone``
* Setting authentication data: ``--registrar_acl_add``
* Creating a price list: ``--price_add``
* Setting a timezone for automated administration: ``--enum_parameter_change``

Detailed manual for init tasks is in :manpage:`fred-admin-inits(1)`.

Cron tasks
^^^^^^^^^^^^^

**Cron tasks** must be set up in :program:`cron` to administer the Registry
regularly.

* Administering registrable objects: ``--object_regular_procedure``
* Updating object states: ``--object_update_states``
* Deleting objects: ``--object_delete_candidates``
* Notifying about object events: ``--notify_state_changes``
* Creating poll messages about request fee: ``--poll_create_request_fee``
* Merging contact duplicates: ``--contact_merge_duplicate_auto``
* Annual contact reminder: ``--contact_reminder``
* Processing public requests: ``--process_public_requests``

Detailed manual for cron tasks is in :manpage:`fred-admin-crons(1)`.
