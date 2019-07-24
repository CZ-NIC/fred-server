
Changing database parameters
----------------------------

Change a value in the database table `enum_parameters`.

.. Important::

   The parameter `regular_day_procedure_zone` must be adapted to your timezone!

   Acceptable values can be found in the Postgres table ``pg_timezone_names``
   (in the `name` column).

Synopsis
^^^^^^^^

``fred-admin --enum_parameter_change
--parameter_name=<string> --parameter_value=<string>``

Options
^^^^^^^^

.. option:: --enum_parameter_change

   The command switch.

.. option:: --parameter_name=<string>

   Selects a parameter to be changed, **mandatory option**.

.. option:: --parameter_value=<string>

   Sets the new value for the parameter, **mandatory option**.

Example
^^^^^^^

``fred-admin --enum_parameter_change
--parameter_name=regular_day_procedure_zone --parameter_value=Chile/EasterIsland``

   Set your timezone for automated administration.
