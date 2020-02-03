
Creating a registrar
-------------------------

Create a new registrar with some data.

See also the concept `Contact merger
<https://fred.nic.cz/documentation/html/Concepts/ContactMerger.html>`_
in the documentation.

Synopsis
^^^^^^^^

``fred-admin --registrar_add --handle=<string> --country=<string> [--city=<string>]
[--dic=<string>] [--email=<email>] [--fax=<string>] [--ico=<string>] [--no_vat]
[--organization=<string>] [--postalcode=<string>] [--reg_name=<string>]
[--stateorprovince=<string>] [--street1=<string>] [--street2=<string>] [--street3=<string>]
[--system] [--telephone=<string>] [--url=<string>] [--varsymb=<string>]``

Options
^^^^^^^^

.. option:: --registrar_add

   The command switch.

.. option:: --city=<string>

   Registrar city.

.. option:: --country=<string>

   Registrar’s country by 2-letter country code (table ``enum_country``), **mandatory option**.

.. option:: --dic=<string>

   Tax identifier number.

.. option:: --email=<email>

   Registrar e-mail.

.. option:: --fax=<string>

   Registrar fax.

.. option:: --handle=<string>

   Handle of the registrar to be added, **mandatory option**.

.. option:: --ico=<string>

   Organization identifier number.

.. option:: --no_vat

   Flag this registrar as NOT a VAT-payer.

.. option:: --organization=<string>

   Registrar’s organization or company.

.. option:: --postalcode=<string>

   Registrar postal code.

.. option:: --reg_name=<string>

   Registrar’s name; you may set it the same as :option:`--organization`.

.. option:: --stateorprovince=<string>

   Registrar state or province.

.. option:: --street1 <string>

   Registrar street #1.

.. option:: --street2 <string>

   Registrar street #2.

.. option:: --street3 <string>

   Registrar street #3.

.. option:: --system

   Designates this registrar to be the “system registrar”.

.. option:: --telephone=<string>

   Registrar telephone.

.. option:: --url=<string>

   Registrar url.

.. option:: --varsymb=<string>

   Registrar variable symbol.

Example
^^^^^^^

``fred-admin --registrar_add --handle=REG-FRED_A --country=CZ
--reg_name="Testing registrar A" --organization="Company l.t.d." --no_vat``

``fred-admin --registrar_add --handle=REG-SYSTEM --country=CZ --system
--reg_name="System registrar"``
