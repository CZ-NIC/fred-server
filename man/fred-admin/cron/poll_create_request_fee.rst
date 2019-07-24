
Poll-messaging about request fee
----------------------------------------

Prepare notifications in the EPP polling about the usage of free EPP requests
and if the registrar exceeded the limit, calculate the price of the requests
over limit.

Cron recommendations
^^^^^^^^^^^^^^^^^^^^

Frequency: once a day (night time)

Synopsis
^^^^^^^^

``fred-admin --poll_create_request_fee [--poll_period_to=<date>]``

Configuration
^^^^^^^^^^^^^

In the database table `request_fee_parameter`.

Options
^^^^^^^^

.. option:: --poll_create_request_fee

   The command switch.

.. option:: --poll_period_to=<date>

   Create poll messages about usage to a given date.

   Default: today

   Refer to the **Generic synopsis** in :manpage:`fred-admin(1)`
   for a date formatting guide.

Example
^^^^^^^^

``fred-admin --poll_create_request_fee --poll_period_to 2019-12-24``

See also
^^^^^^^^

See also the concept `Communication
<https://fred.nic.cz/documentation/html/Concepts/Communication.html>`_
in the documentation.
