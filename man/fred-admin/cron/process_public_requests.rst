
Processing public requests
--------------------------

Process resolved public requests -- compose and send emails with listing
of recorded personal data to contacts that have requested it.

This task will generate emails in response to resolved public requests of the
following types:

* ``personalinfo_auto_pif`` – requests to send personal info to an email
  in the Registry,
* ``personalinfo_email_pif`` – requests to send personal info to another email,
  authorized with an email signed with a digital signature,
* ``personalinfo_post_pif`` – requests to send personal info to another email,
  authorized with a letter containing a notarized signature.

.. Note:: This command processes only requests for personal information.

Cron recommendations
^^^^^^^^^^^^^^^^^^^^

Frequency: every 5 minutes

Synopsis
^^^^^^^^

``fred-admin --process_public_requests [--types=<list>]``

Options
^^^^^^^^

.. option:: --process_public_requests

   The command switch.

.. option:: --types=<list of types>

   Process only the listed types (see above).

   Default: all 3 types (`personalinfo_*` types only!)

Example
^^^^^^^

``fred-admin --process_public_requests --types personalinfo_auto_pif``

See also
^^^^^^^^

See the concept `Public requests <https://fred.nic.cz/documentation/html/Concepts/UsersInterfaces.html#public-requests>`_ in the documentation.
