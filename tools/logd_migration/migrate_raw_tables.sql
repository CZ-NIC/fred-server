-- dependent on log_partitioning_function.sql
-- works until the year 2099 :)

create or replace function migrate_login() returns void as $migrate_login$
declare
	l_start 	timestamp without time zone;
	l_end		timestamp without time zone;
	terminal_date 	timestamp without time zone;
	next_tbegin 	timestamp without time zone;
	t_begin 	timestamp without time zone;
	curr_id 	integer;

begin
	raise notice 'Migrating the login table into session. ';

	-- set the current value of the sequence for session

	select max(logindate) into l_start from login;
	select min(logindate) into l_end   from login;

	if ( to_char (l_start, 'YY_MM') = to_char (l_end, 'YY_MM')) then
		perform fill_part_session(l_end, l_start);
		return;
	end if;

	terminal_date := date_trunc('month', l_end);

	next_tbegin := date_trunc ('month', l_start);

	perform fill_part_session(next_tbegin, l_start);
		
	loop
		t_begin := next_tbegin;
		next_tbegin := t_begin - interval '1 month';
		exit when next_tbegin = terminal_date;

		perform fill_part_session(next_tbegin, t_begin);
	end loop;

	perform fill_part_session(terminal_date, t_begin);

	select max(id) into curr_id from session;
	perform setval('session_id_seq'::regclass, curr_id +1);

end;
$migrate_login$ language plpgsql;


create or replace function migrate_raw_data(date_low timestamp without time zone, date_high timestamp without time zone) returns void as $migrate_raw_data$
begin

--       set maintenance_work_mem=262144;
--       set work_mem=262144;
--       set synchronous_commit=off;

	perform migrate_raw_action(date_low, date_high);
	perform migrate_raw_action_xml(date_low, date_high);
	perform migrate_login();


end;
$migrate_raw_data$ language plpgsql;

create or replace function migrate_raw_action(m_start timestamp without time zone, m_end timestamp without time zone) returns void as $migrate_raw_action$
DECLARE
	terminal_date 	timestamp without time zone;
	t_begin 	timestamp without time zone;
	next_tbegin 	timestamp without time zone;
	curr_id 	integer;
BEGIN

	if ( to_char (m_start, 'YY_MM') = to_char (m_end, 'YY_MM')) then
		perform fill_part_request(m_start, m_end);
		return;
	end if;

	terminal_date := date_trunc('month', m_end);
	next_tbegin := date_trunc('month', m_start) + interval '1 month';

	perform fill_part_request(m_start, next_tbegin);
	
	loop
		t_begin := next_tbegin;
		exit when t_begin = terminal_date;
		next_tbegin := t_begin + interval '1 month';
		perform fill_part_request(t_begin, next_tbegin);
	end loop;

	perform fill_part_request(terminal_date, m_end);

	select max(id) into curr_id from request;
	perform setval('request_id_seq'::regclass, curr_id +1);	
END;
$migrate_raw_action$ language plpgsql;

create or replace function migrate_raw_action_xml(m_start timestamp without time zone, m_end timestamp without time zone) returns void as $migrate_raw_action_xml$
DECLARE
	terminal_date 	timestamp without time zone;
	t_begin 	timestamp without time zone;
	next_tbegin 	timestamp without time zone;
	curr_id 	integer;
BEGIN

	if ( to_char (m_start, 'YY_MM') = to_char (m_end, 'YY_MM')) then
		perform fill_part_request_data(m_start, m_end);
		return;
	end if;

	terminal_date := date_trunc('month', m_end);
	next_tbegin := date_trunc('month', m_start) + interval '1 month';

	perform fill_part_request_data(m_start, next_tbegin);
	
	loop
		t_begin := next_tbegin;
		exit when t_begin = terminal_date;
		next_tbegin := t_begin + interval '1 month';
		perform fill_part_request_data(t_begin, next_tbegin);
	end loop;

	perform fill_part_request_data(terminal_date, m_end);

	select max(id) into curr_id from request;
	perform setval('request_id_seq'::regclass, curr_id +1);	
END;
$migrate_raw_action_xml$ language plpgsql;


create or replace function fill_part_request(tbegin timestamp without time zone, tend timestamp without time zone) returns void as $fill_part_request$
declare 
	table_name varchar(30);
	create_table 	text;
	spec_alter_table text;
	stmt text;
	table_count integer;

	lower timestamp without time zone;
	upper  timestamp without time zone;
	
BEGIN

	table_name := 'request_' || partition_postfix(tbegin, 3, false);

	lower := to_char(date_trunc('month', tbegin), 'YYYY-MM-DD');
	upper := to_char(date_trunc('month', tbegin + interval '1 month'), 'YYYY-MM-DD');

        -- the table shouldn't exist - otherwise we have to drop all the keys etc..
	-- select count(*) into table_count from pg_tables where tablename=table_name;
	-- if (table_count = 0) then

                create_table := 'CREATE TABLE ' || table_name || '    (CHECK (time_begin >= TIMESTAMP ''' || lower || ''' and time_begin < TIMESTAMP ''' 
		|| upper || ''' AND service = 3 AND is_monitoring = false) ) INHERITS (request)';  	

                execute create_table;
		-- perform create_tbl_request(tbegin, 3, false);
	-- end if;

	stmt := 'insert into ' || table_name || ' (id, time_begin, time_end, source_ip, service, action_type, session_id, user_name, is_monitoring) select a.id, startdate, enddate, null, 3, action, clientid, r.handle, false from action a join login l on a.clientid = l.id join registrar r on r.id = l.registrarid where startdate >= ''' || tbegin || ''' and startdate <= ''' || tend || ''' and clienttrid != ''monitoring''';
	execute stmt;

        -- statement for records without clientid (reference to login)
        stmt := 'insert into ' || table_name || ' (id, time_begin, time_end, source_ip, service, action_type, session_id, is_monitoring) select id, startdate, enddate, null, 3, action, clientid, false from action where startdate >= ''' || tbegin || ''' and startdate <= ''' || tend || ''' and clientid is null and clienttrid != ''monitoring''';
        execute stmt;

        -- add the rest of the statements
	spec_alter_table := 'ALTER TABLE ' || table_name || ' ADD PRIMARY KEY (id); ';
        execute spec_alter_table;

	perform create_indexes_request(table_name);


        ----- ok, now with monitoring


        table_name := 'request_' || partition_postfix(tbegin, 3, true);

--	select count(*) into table_count from pg_tables where tablename=table_name;
--	if (table_count = 0) then

                create_table := 'CREATE TABLE ' || table_name || '    (CHECK (time_begin >= TIMESTAMP ''' || lower || ''' and time_begin < TIMESTAMP ''' 
		|| upper || ''' AND is_monitoring = true) ) INHERITS (request)';  	

                execute create_table;
--		perform create_tbl_request(tbegin, 3, true);
--	end if;

	stmt := 'insert into ' || table_name || ' (id, time_begin, time_end, source_ip, service, action_type, session_id, user_name, is_monitoring) select a.id, startdate, enddate, null, 3, action, clientid, r.handle, true from action a join login l on a.clientid = l.id join registrar r on r.id = l.registrarid where startdate >= ''' || tbegin || ''' and startdate <= ''' || tend || ''' and clienttrid = ''monitoring''';
	execute stmt;

	stmt := 'insert into ' || table_name || ' (id, time_begin, time_end, source_ip, service, action_type, session_id, is_monitoring) select id, startdate, enddate, null, 3, action, clientid, true from action where startdate >= ''' || tbegin || ''' and startdate <= ''' || tend || ''' and clientid is null and clienttrid = ''monitoring''';
	execute stmt;

        -- add the rest of the statements
        spec_alter_table := 'ALTER TABLE ' || table_name || ' ADD PRIMARY KEY (id); ';
        execute spec_alter_table;

	perform create_indexes_request(table_name);

END;
$fill_part_request$ language plpgsql;


create or replace function fill_part_request_data(tbegin timestamp without time zone, tend timestamp without time zone) returns void as $fill_part_request_data$
declare 
	table_name varchar(30);
        table_postfix varchar(40);
	stmt            text;
        create_table    text;
        spec_alter_table text;
	table_count integer;
	lower timestamp without time zone;
	upper  timestamp without time zone;
	
BEGIN
        table_postfix := partition_postfix(tbegin, 3, false);
	table_name := 'request_data_' || table_postfix;

	lower := to_char(date_trunc('month', tbegin), 'YYYY-MM-DD');
	upper := to_char(date_trunc('month', tbegin + interval '1 month'), 'YYYY-MM-DD');

	-- select count(*) into table_count from pg_tables where tablename=table_name;
	-- if (table_count = 0) then
            create_table  =  'CREATE TABLE ' || table_name || ' (CHECK (entry_time_begin >= TIMESTAMP ''' || lower || ''' and entry_time_begin < TIMESTAMP ''' || upper || ''' AND entry_service = 3 AND entry_monitoring = false) ) INHERITS (request_data) ';
            execute create_table;

	--	perform create_tbl_request_data(tbegin, 3, false);
	-- end if;

	stmt := 'insert into ' || table_name || ' (entry_time_begin, entry_service, entry_monitoring, entry_id , content, is_response) select a.time_begin, 3, a.is_monitoring, actionid, xml, false from action_xml join request a on a.id=actionid where a.time_begin >= ''' || tbegin || ''' and a.time_begin <= ''' || tend || ''' and xml is not null and a.is_monitoring=false';
	execute stmt;
	spec_alter_table = 'ALTER TABLE ' || table_name || ' ADD CONSTRAINT ' || table_name || '_entry_id_fkey FOREIGN KEY (entry_id) REFERENCES request_' || table_postfix || '(id); ';
        execute spec_alter_table;
	perform create_indexes_request_data(table_name);

        ------ monitoring 
        table_postfix := partition_postfix(tbegin, 3, true);
	table_name := 'request_data_' || table_postfix;

	-- select count(*) into table_count from pg_tables where tablename=table_name;
	-- if (table_count = 0) then

            create_table  =  'CREATE TABLE ' || table_name || ' (CHECK (entry_time_begin >= TIMESTAMP ''' || lower || ''' and entry_time_begin < TIMESTAMP ''' || upper || ''' AND entry_monitoring = true) ) INHERITS (request_data) ';	
            execute create_table;

	--	perform create_tbl_request_data(tbegin, 3, true);
	-- end if;
	stmt := 'insert into ' || table_name || ' (entry_time_begin, entry_service, entry_monitoring, entry_id , content, is_response) select a.time_begin, 3, a.is_monitoring, actionid, xml, true from action_xml join request a on a.id=actionid where a.time_begin >= ''' || tbegin || ''' and a.time_begin <= ''' || tend || ''' and xml is not null and a.is_monitoring=true';
	execute stmt;
	spec_alter_table = 'ALTER TABLE ' || table_name || ' ADD CONSTRAINT ' || table_name || '_entry_id_fkey FOREIGN KEY (entry_id) REFERENCES request_' || table_postfix || '(id); ';
        execute spec_alter_table;
	perform create_indexes_request_data(table_name);

END;
$fill_part_request_data$ language plpgsql;

create or replace function fill_part_session(tbegin timestamp without time zone, tend timestamp without time zone) returns void as $fill_part_session$
declare
	table_count integer;
	table_name varchar(30);
        create_table text;
	stmt text;
        spec_alter_table text;
	lower timestamp without time zone;
	upper  timestamp without time zone;
begin
	table_name := 'session_' || partition_postfix(tbegin, -1, false); 

	lower := to_char(date_trunc('month', tbegin), 'YYYY-MM-DD');
	upper := to_char(date_trunc('month', tbegin + interval '1 month'), 'YYYY-MM-DD');

	-- select count(*) into table_count from pg_tables where tablename=table_name;
	-- if(table_count = 0) then

	create_table :=  'CREATE TABLE ' || table_name || '    (CHECK (login_date >= TIMESTAMP ''' || lower || ''' and login_date < TIMESTAMP ''' || upper || ''') ) INHERITS (session) ';
        execute create_table;

	--	perform create_tbl_session(tbegin);
	-- end if;
	stmt := 'insert into ' || table_name || ' (id, name, login_date, logout_date, lang) select id, registrarid, logindate, logoutdate, lang from login where logindate >= ''' || tbegin || ''' and logindate <= ''' || tend || ''' ';
	execute stmt;
        

	spec_alter_table := 'ALTER TABLE ' || table_name || ' ADD PRIMARY KEY (id); ';
        execute spec_alter_table;
	perform create_indexes_session(table_name);

end;
$fill_part_session$ language plpgsql;



