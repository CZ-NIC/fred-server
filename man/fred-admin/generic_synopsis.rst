
Generic synopsis
----------------

**fred-admin** <*command-switch*> [*options*]

Common options
^^^^^^^^^^^^^^

.. option:: -C <file>, --config <file>

   Load configuration from this file.

   Default: /etc/fred/server.conf

.. option:: -h, --help

   Display program help.

Option syntax
^^^^^^^^^^^^^

Options can be given in 2 equivalent syntaxes:

   * `--option_name option_parameter` (space-separated)
   * `--option_name=option_parameter` (separated with the equals sign)

Date and time formats
^^^^^^^^^^^^^^^^^^^^^

Some options accept date or datetime values, which must be formatted as follows:

- date format: `YYYY-MM-DD`
- datetime format: `YYYY-MM-DD HH:mm:ss`

*Ranges* can be input as follows:

- ``--crdate="2019-10-16;2019-10-20"``

  Between these days (including the days).
- ``--crdate="2019-10-16;"``

  This whole day or later.
- ``--crdate=";2019-10-16"``

  This whole day or earlier.
- ``--crdate="2019-10-16"`` or ``--crdate="2019-10"`` or ``--crdate="2019"``

  This whole day or month or year.
- ``--crdate="last_week;-1"``

  In the previous week.

  The following *relative ranges* are accepted: ``last_day``,
  ``last_week``, ``last_month``, ``last_year``, ``past_hour``, ``past_week``,
  ``past_month``, ``past_year``

  last_INTERVAL
     the range of the INTERVAL with the beginning of the INTERVAL
     shifted by the offset of the same INTERVAL
     |br|
     e.g. last_week;-3 = a week three weeks ago (spans 1 week)
  past_INTERVAL
     the range from the current time to the time that is defined by the INTERVAL
     multiplied by the offset
     |br|
     e.g. past_week;-3 = over the range of last three weeks (spans 3 weeks)
