
Granting access to a zone
-------------------------

Grant a registrar permissions to manage objects in a specified zone.

Synopsis
^^^^^^^^

``fred-admin --registrar_add_zone --handle=<string>
--zone_fqdn=<string> [--from_date=<date>]``

Options
^^^^^^^^

.. option:: --registrar_add_zone

   The command switch.

.. option:: --from_date=<date>

   Date since when the access is allowed.

   Default: today

   Refer to the **Generic synopsis** in :manpage:`fred-admin(1)`
   for a date formatting guide.

.. option:: --handle=<string>

   Registrar's handle, **mandatory option**.

.. option:: --zone_fqdn=<string>

   Name of a zone the registrar gains access to, **mandatory option**.

Example
^^^^^^^

``fred-admin --registrar_add_zone --handle=REG-FRED_A --zone_fqdn=cz --from_date=2020-01-06``
