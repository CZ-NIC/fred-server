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
	perform migrate_raw_action(date_low, date_high);
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

	--select startdate into m_start from action where id = id_low;
	--select startdate into m_end   from action where id = id_high;


	if ( to_char (m_start, 'YY_MM') = to_char (m_end, 'YY_MM')) then
		perform fill_part(m_start, m_end);
		return;
	end if;

	terminal_date := date_trunc('month', m_end);
	next_tbegin := date_trunc('month', m_start) + interval '1 month';

	perform fill_part(m_start, next_tbegin);
	
	loop
		t_begin := next_tbegin;
		exit when t_begin = terminal_date;
		next_tbegin := t_begin + interval '1 month';
		perform fill_part(t_begin, next_tbegin);
	end loop;

	perform fill_part(terminal_date, m_end);

	select max(id) into curr_id from request;
	perform setval('request_id_seq'::regclass, curr_id +1);	
END;
$migrate_raw_action$ language plpgsql;


create or replace function fill_part(tbegin timestamp without time zone, tend timestamp without time zone) returns void as $fill_part$
declare
begin
	perform fill_part_request(tbegin, tend, 3);
	perform fill_part_request_data(tbegin, tend, 3);
end;
$fill_part$ language plpgsql;


-- TODO add session ID
create or replace function fill_part_request(tbegin timestamp without time zone, tend timestamp without time zone, service integer) returns void as $fill_part_request$
declare 
	table_name varchar(30);
	stmt text;
	table_count integer;
	
BEGIN
	-- table_name := 'request_' || to_char (tbegin, 'YY_MM');
	table_name := 'request_' || partition_postfix(tbegin, service, false);

	select count(*) into table_count from pg_tables where tablename=table_name;
	if (table_count = 0) then
		perform create_tbl_request(tbegin, service, false);
	end if;

	stmt := 'insert into ' || table_name || ' (id, time_begin, time_end, source_ip, service, action_type, session_id, is_monitoring) select id, startdate, enddate, null, 3, action, clientid, false from action where startdate >= ''' || tbegin || ''' and startdate < ''' || tend || '''';

	execute stmt;
END;
$fill_part_request$ language plpgsql;


create or replace function fill_part_request_data(tbegin timestamp without time zone, tend timestamp without time zone, service integer) returns void as $fill_part_request_data$
declare 
	table_name varchar(30);
	stmt text;
	table_count integer;
	rec record;
	
BEGIN
--	table_name := 'request_data_' || to_char (tbegin, 'YY_MM');
	table_name := 'request_data_' || partition_postfix(tbegin, service, false);

	select count(*) into table_count from pg_tables where tablename=table_name;
	if (table_count = 0) then
		perform create_tbl_request_data(tbegin, service, false);
	end if;

	stmt := 'insert into ' || table_name || ' (entry_time_begin, entry_service, entry_monitoring, entry_id , content, is_response) select a.startdate, 3, false, actionid, xml, false from action_xml join action a on a.id=actionid where a.startdate >= ''' || tbegin || ''' and a.startdate < ''' || tend || ''' and xml is not null';
	execute stmt;

END;
$fill_part_request_data$ language plpgsql;

create or replace function fill_part_session(tbegin timestamp without time zone, tend timestamp without time zone) returns void as $fill_part_session$
declare
	table_count integer;
	table_name varchar(30);
	stmt text;
begin
	table_name := 'session_' || partition_postfix(tbegin, -1, false); 

	select count(*) into table_count from pg_tables where tablename=table_name;
	if(table_count = 0) then
		perform create_tbl_session(tbegin);
	end if;
	
	raise notice 'This is some wicked value: % ', tbegin;
	
	stmt := 'insert into ' || table_name || ' (id, name, login_date, logout_date, lang) select id, registrarid, logindate, logoutdate, lang from login where logindate >= ''' || tbegin || ''' and logindate < ''' || tend || ''' ';
	execute stmt;

end;
$fill_part_session$ language plpgsql;



