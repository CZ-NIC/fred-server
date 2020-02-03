
Creating a price list
---------------------

Add a price of an operation in a zone valid in a given time span.
The amount is currency-independent, decimals are allowed.
If you don’t want to charge for an operation, just set the price to zero.

Synopsis
^^^^^^^^

``fred-admin --price_add --operation=<string> --zone_fqdn=<string>
--operation_price=<decimal> [--period=<integer>] [--enable_postpaid_operation]
[--valid_from=<datetime>] [--valid_to=<datetime>]``

Options
^^^^^^^^

.. option:: --price_add

   The command switch.

.. option:: --enable_postpaid_operation

   Operation charge doesn’t require prepaid credit (allows negative credit).

.. option:: --operation=<string>

   Charged operation, **mandatory option**.

   Chargeable operations include:

   - `CreateDomain` -- domain creation (one-time payment when a new domain
     is introduced to the Registry).

     Pricing period: one-time payment

   - `RenewDomain` -- domain renewal (renewal per unit).

     Pricing period: per unit
     (defined by `ex_period_min` in :option:`--zone_add`)

   - `GeneralEppOperation` -- operation over request-usage limit (charged only
     after all uncharged requests were exhausted).

     Pricing period: per operation

.. option:: --operation_price=<decimal>

   Amount, e.g. 140.00, **mandatory option**.

.. option:: --period=<integer>

   Pricing period/quantity.

   Default: 1

.. option:: --valid_from, --valid_to=<datetime>

    Range of UTC datetimes when the pricing scheme will be used;
    `valid_from < valid_to`.

    Refer to the **Generic synopsis** in :manpage:`fred-admin(1)`
    for a date formatting guide.

.. option:: --zone_fqdn=<string>

   Zone FQDN, **mandatory option**.

Example
^^^^^^^

``fred-admin --price_add --operation=CreateDomain --zone_fqdn=cz
--operation_price 0 --period 1 --valid_from='2018-12-31 23:00:00'``

``fred-admin --price_add --operation=RenewDomain --zone_fqdn=cz
--operation_price 155 --period 1
--valid_from='2018-12-31 23:00:00' --valid_to='2019-01-31 22:59:59'``

``fred-admin --price_add --operation=GeneralEppOperation --zone_fqdn=cz
--operation_price 0.10 --period 1 --enable_postpaid_operation
--valid_from='2018-12-31 23:00:00'``
