
Adding a zone
-------------

Create a new zone in the Registry.

It does not have to be only a TLD zone, it can also be e.g. **go.to** or an ENUM
domain (like **0.2.4.e164.arpa**).

Consider thoroughly which parameters you will set, there is no command to edit
zones later.

Synopsis
^^^^^^^^

``fred-admin --zone_add --zone_fqdn=<string> [--ex_period_min=<integer>]
[--ex_period_max=<integer>] [--expiry=<integer>] [--hostmaster=<email>]
[--minimum=<integer>] [--ns_fqdn=<string>] [--refresh=<integer>]
[--ttl=<integer>] [--update_retr=<integer>]``

Options
^^^^^^^^

.. option:: --zone_add

   The command switch.

.. option:: --ex_period_min=<integer>

   Minimum number of **months**, for which a domain in the zone can be
   registered.

   The ``ex_period_min`` number is also used as a unit for registration periods
   which are then defined as multiples of this number, i.e. with ``--ex_period_min=12``
   domains can be registered (and renewed) for whole years, not e.g. year and half.

   Default: 12 [months]

.. option:: --ex_period_max=<integer>

   Maximum number of **months**, for which a domain in the zone can be
   registered.

   Default: 120 [months]

.. option:: --expiry=<integer>

   Zone expiration period for secondary nameservers (zone-file SOA record).

   Default: 604800 [s]

.. option:: --hostmaster=<email>

   Mailbox of the person responsible for this zone (zone-file SOA record);
   it can be given either in email syntax (\ `user@hostname`) or RNAME
   syntax (\ `user.hostname`).

   Default: hostmaster\@localhost

.. option:: --minimum=<integer>

   The period for which a result of a non-existent domain should
   be cached by resolvers (zone-file SOA record).

   Default: 900 [s]

.. option:: --ns_fqdn=<string>

   Fully Qualified Domain Name of the nameserver (zone-file SOA record).

   Default: localhost

.. option:: --refresh=<integer>

   Secondary nameservers copy of zone refresh (zone-file SOA record).

   Default: 900 [s]

.. option:: --ttl=<integer>

   Time to live - the default validity period of the resource records
   in the zone (zone-file SOA record).

   Default: 18000 [s]

.. option:: --update_retr=<integer>

   Retry interval of zone update for secondary nameservers in case of
   failed zone refresh (zone-file SOA record).

   Default: 300 [s]

.. option:: --zone_fqdn=<string>

   FQDN of the zone to be added, **mandatory option**.

Example
^^^^^^^

``fred-admin --zone_add --zone_fqdn=cz``

``fred-admin --zone_add --zone_fqdn=cz --ex_period_min=12 --ex_period_max=120
--ttl=18000 --hostmaster=admin@registry.cz --refresh=900
--update_retr=300 --expiry=604800 --minimum=900 --ns_fqdn=localhost``
