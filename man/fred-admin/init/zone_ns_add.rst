
Adding zone nameservers
-------------------------

Assign a nameserver to a zone.

Synopsis
^^^^^^^^

``fred-admin --zone_ns_add --zone_fqdn=<string> --ns_fqdn=<string> [--addr=<string>]``

Options
^^^^^^^^

.. option:: --zone_ns_add

   The command switch.

.. option:: --addr=<string>

   Nameserver's IP address (glue) - required when the nameserver's FQDN is from the same zone
   to which it is being added; you can list several IP addresses separated with a space.

.. option:: --ns_fqdn=<string>

   Nameserver's Fully Qualified Domain Name (FQDN), **mandatory option**.

.. option:: --zone_fqdn=<string>

   The zone a nameserver is added to, **mandatory option**.

Example
^^^^^^^

``fred-admin --zone_ns_add --zone_fqdn=cz --ns_fqdn=a.ns.nic.cz --addr=1.2.3.4``
