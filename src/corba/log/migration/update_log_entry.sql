create or replace function update_log_entry(log_begin integer, log_end integer, action_begin integer, action_end integer) returns void as $$

declare 
	cur_log refcursor;
	cur_action refcursor;
	-- TODO please edit : this is id of name 'sessionId' in log_property_name table
	SESSION_NAME integer := 15;

	id_log integer;
	id_action integer;
	sessionid integer;
	action_t integer;

	tb timestamp;
	te timestamp;
	ipaddr INET;
	act_entry_id integer;

begin

	open cur_log for select id from log_entry where id>=log_begin and id<= log_end order by id;

	open cur_action for select actionid from action_xml where actionid >= action_begin and actionid <= action_end order by actionid; 

	while true loop

		fetch cur_log into id_log;
		if not found then
			exit;
		end if;

		fetch cur_action into id_action;
		if not found then
			exit;
		end if;
			
		update log_entry set (time_begin, time_end, source_ip) = (
		  (select startdate from action where id = id_action),
		  (select enddate from action where id = id_action),
			null) where id = id_log;

		select clientid into sessionid from action where id=id_action;
		if sessionid is not null then
			insert into log_property_value (entry_time_begin, entry_id, name_id, value, output) values ((select time_begin from log_entry where id = id_log), id_log, SESSION_NAME, sessionid, false);
		end if;

	-- TODO sessionid

	-- TEMP
	--	if (select xml_out from action_xml where actionid=id_action) is not null then
	--		insert into log_raw_content (entry_id, content, is_response) values (id_log, (select substring(xml_out from 0 for 2000) from action_xml where actionid=id_action), true);
	--	end if;

	end loop;

	close cur_log;
	close cur_action;

	-- now transfer all the records from action without corresponding action_xml records
	open cur_action for select startdate, enddate, clientid, action from action where id>=action_begin and id<=action_end and not exists (select * from action_xml where actionid=id);

	while true loop
		fetch cur_action into tb, te, sessionid, action_t;
		if not found then 
			exit;
		end if;

		-- TODO it doesn't know if it's monitoring - insert false 
		insert into log_entry (time_begin, time_end, source_ip, service, action_type, is_monitoring) values (tb, te, null, 3, action_t, false);

		if sessionid is not null then

			select currval('log_entry_id_seq'::regclass) into act_entry_id;
			insert into log_property_value (entry_time_begin, entry_id, name_id, value, output) values ((select time_begin from log_entry where id=act_entry_id), act_entry_id, SESSION_NAME, sessionid, false);
		end if;
	end loop;	

	close cur_action;

	-- now migrate the login table
	insert into log_session (id, name, login_trid, login_date, logout_trid, logout_date, lang) select id, registrarid, logintrid, logindate, logouttrid, logoutdate, lang from login;


end;
$$ language plpgsql;

