-- dependent on log_partitioning_function.sql
-- works until the year 2099 :)

create or replace function migrate_raw_data(id_low integer, id_high integer) returns void as $migrate_raw_data$
DECLARE
	terminal_date 	timestamp without time zone;
	m_start 	timestamp without time zone;
 	m_end 		timestamp without time zone;
	t_begin 	timestamp without time zone;
	next_tbegin 	timestamp without time zone;
BEGIN

	select startdate into m_start from action where id = id_low;
	select startdate into m_end   from action where id = id_high;

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
END;
$migrate_raw_data$ language plpgsql;


create or replace function fill_part(tbegin timestamp without time zone, tend timestamp without time zone) returns void as $fill_part$
begin
	perform fill_part_log_entry(tbegin, tend);
	perform fill_part_log_raw_content(tbegin, tend);
end;
$fill_part$ language plpgsql;

create or replace function fill_part_log_entry(tbegin timestamp without time zone, tend timestamp without time zone) returns void as $fill_part_log_entry$
declare 
	table_name varchar(30);
	stmt text;
	table_count integer;
	rec record;
	
BEGIN
	table_name := 'log_entry_' || to_char (tbegin, 'YY_MM');

	select count(*) into table_count from pg_tables where tablename=table_name;

	raise notice 'Table count: %', table_count;

	if (table_count = 0) then
		perform create_log_entry(tbegin, 0);
	end if;



	stmt := 'insert into ' || table_name || ' (id, time_begin, time_end, source_ip, service, action_type, is_monitoring) select id, startdate, enddate, null, 3, action, false from action where startdate >= ''' || tbegin || ''' and startdate < ''' || tend || '''';

	execute stmt;
END;
$fill_part_log_entry$ language plpgsql;


create or replace function fill_part_log_raw_content(tbegin timestamp without time zone, tend timestamp without time zone) returns void as $fill_part_log_raw_content$
declare 
	table_name varchar(30);
	stmt text;
	table_count integer;
	rec record;
	
BEGIN
	table_name := 'log_raw_content_' || to_char (tbegin, 'YY_MM');

	select count(*) into table_count from pg_tables where tablename=table_name;
	if (table_count = 0) then
		perform create_log_raw_content(tbegin, 0);
	end if;

	stmt := 'insert into ' || table_name || ' (entry_time_begin, entry_id , content, is_response) select a.startdate, actionid, xml, false from action_xml join action a on a.id=actionid where a.startdate >= ''' || tbegin || ''' and a.startdate < ''' || tend || ''' and xml is not null';
	execute stmt;
	-- raise notice 'Statement false: %', stmt;

	stmt := 'insert into ' || table_name || ' (entry_time_begin, entry_id , content, is_response) select a.startdate, actionid, xml_out, true from action_xml join action a on a.id=actionid where a.startdate >= ''' || tbegin || ''' and a.startdate < ''' || tend || ''' and xml_out is not null';
	execute stmt;
	-- raise notice 'Statement true : %', stmt;
END;
$fill_part_log_raw_content$ language plpgsql;


