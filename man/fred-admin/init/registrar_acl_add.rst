
Setting authentication data
------------------------------

Assign the given access control data to a registrar.

Authentication data allows registrars to connect to the Registry securely.

Synopsis
^^^^^^^^

``fred-admin --registrar_acl_add --handle=<string>
--certificate=<string> --password=<string>``

Options
^^^^^^^^

.. option:: --registrar_acl_add

   The command switch.

.. option:: --certificate=<string>

   Fingerprint of the registrar’s certificate, **mandatory option**.

   Can be created from an existing certificate with the following command::

      openssl x509 -noout -fingerprint -md5 -in /path/to/cert.pem | cut -d= -f2

   For testing purposes, you can use the test certificate that comes with the
   ``fred-mod-eppd`` package and was installed in
   ``$PREFIX/share/fred-mod-eppd/ssl/``.

.. option:: --handle=<string>

   Registrar's handle, **mandatory option**.

.. option:: --password=<string>

   Registrar’s password, **mandatory option**.

   Both the password and certificate are needed to access the Registry.

Example
^^^^^^^

``fred-admin --registrar_acl_add --handle=REG-FRED_A
--certificate="39:D1:0C:CA:05:3A:CC:C0:0B:EC:6F:3F:81:0D:C7:9E" --password=passwd``
