
Merging contact duplicates
--------------------------

Merge duplicate contact records automatically. This involves:

- Lookup of duplicate contacts per registrar.
- Selection of the best destination contact.
- Merging all source contacts into the destination contact
  (source contacts are deleted).

Cron recommendations
^^^^^^^^^^^^^^^^^^^^

Frequency: once a week

Synopsis
^^^^^^^^

``fred-admin --contact_merge_duplicate_auto [--dry_run] [--except_registrar=<handle> ... | --registrar=<handle> ...] [--selection_filter_order=<list>]``

Options
^^^^^^^^

.. option:: --contact_merge_duplicate_auto

   The command switch.

.. option:: --dry_run

   Preview what would be done while doing nothing.

   Default: off

.. option:: --except_registrar=<handle>

   Exclude this registrar when looking for duplicates; can be used repeatedly
   to exclude more registrars.

   **It is recommended to exclude the system registrar.**

   Default: none

.. option:: --registrar=<handle>

   Include this registrar when looking for duplicates; can be used repeatedly
   to include more registrars.

   Default: all

.. option:: --selection_filter_order=<ordered list of filters>

   Change the use and/or priority of filters for selection of the destination
   contact.

   Filters:

   - `mcs_filter_max_domains_bound` – contact has most domains linked
     as a holder or administrative contact
   - `mcs_filter_max_objects_bound` – contact has most objects linked
     (domains, nssets or keysets)
   - `mcs_filter_recently_updated` – contact has been updated most recently
   - `mcs_filter_recently_created` – contact has been created most recently

   Default: mcs_filter_max_domains_bound,mcs_filter_max_objects_bound,mcs_filter_recently_updated,mcs_filter_recently_created

Example
^^^^^^^^

``fred-admin --contact_merge_duplicate_auto --except_registrar REG-SYSTEM --selection_filter_order mcs_filter_max_domains_bound,mcs_filter_recently_created``

``fred-admin --contact_merge_duplicate_auto --registrar REG-RRR --registrar REG-SSS --dry_run``

See also
^^^^^^^^

See also the concept `Contact merger
<https://fred.nic.cz/documentation/html/Concepts/ContactMerger.html>`_
in the documentation.
