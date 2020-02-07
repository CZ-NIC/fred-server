
Annual contact reminder
-----------------------

Send emails to remind contacts to review their contact details
and to inform them about objects linked to their contact.

This task will select contacts that:

- are linked to other objects,
- were created on the day and month 300 days ago (before the specified date),
- were not changed in the last 300 days (relatively to the specified date).

And send them an email of the `annual_contact_reminder` type.

Cron recommendations
^^^^^^^^^^^^^^^^^^^^

Frequency: once a day

Synopsis
^^^^^^^^

``fred-admin --contact_reminder [--date=<date>]``

Options
^^^^^^^^

.. option:: --contact_reminder

   The command switch.

.. option:: --date=<date>

   Specify a different date for contact selection (useful for backdating).

   Default: today

   Refer to the **Generic synopsis** in :manpage:`fred-admin(1)`
   for a date formatting guide.

Example
^^^^^^^^

``fred-admin --contact_reminder --date 2019-07-22``

See also
^^^^^^^^

See also `Email type: annual_contact_reminder <https://fred.nic.cz/documentation/html/AdminManual/Appendixes/EmailParameters.html#email-type-contact-reminder>`_ in the documentation.
