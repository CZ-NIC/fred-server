 SELECT r.user_name, COUNT(*)
   FROM request r
   LEFT JOIN (
            request_property_value rpv
       JOIN request_property_name rpn
         ON rpn.id = rpv.property_name_id
    AND rpn.name = 'handle'
        )
     ON (
            rpv.request_id = r.id
        AND rpv.request_service_id = $1::INTEGER
        AND rpv.request_time_begin >= $2::timestamp
        AND rpv.request_time_begin < $3::TIMESTAMP
    AND rpv.request_monitoring = FALSE
        )
   JOIN result_code rc
     ON rc.id = r.result_code_id
    AND rc.name NOT IN ('CommandFailed', 'CommandFailedServerClosingConnection')
   JOIN request_type rt
     ON rt.id = r.request_type_id
    AND rt.name NOT IN ('PollAcknowledgement', 'PollResponse')
  WHERE r.service_id = $1::INTEGER
    AND r.time_begin >= $2::TIMESTAMP
    AND r.time_begin < $3::TIMESTAMP
    AND r.is_monitoring = FALSE
  GROUP BY r.user_name
  ORDER BY r.user_name;

SELECT r.user_name, COUNT(*)
  FROM request r
  LEFT JOIN (
           request_property_value rpv
      JOIN request_property_name rpn ON rpn.id = rpv.property_name_id
       AND rpn.name = 'handle'
       ) ON (
           rpv.request_id = r.id
       AND rpv.request_service_id = $1::INTEGER
       AND rpv.request_time_begin >= $2::TIMESTAMP
       AND rpv.request_time_begin < $3::TIMESTAMP
       AND rpv.request_monitoring = FALSE
       )
  JOIN result_code rc ON rc.id = r.result_code_id
   AND rc.name NOT IN ('CommandFailed', 'CommandFailedServerClosingConnection')
  JOIN request_type rt ON rt.id = r.request_type_id
   AND rt.name NOT IN ('PollAcknowledgement', 'PollResponse')
 WHERE r.service_id = $1::INTEGER
   AND r.time_begin >= $2::TIMESTAMP
   AND r.time_begin < $3::TIMESTAMP
   AND r.is_monitoring = FALSE
 GROUP BY r.user_name
 ORDER BY r.user_name;



SELECT r.user_name, COUNT(*)
  FROM request r
  LEFT JOIN request_property_value rpv ON rpv.request_id = r.id
   AND rpv.request_service_id = $1::INTEGER
   AND rpv.request_time_begin >= $2::TIMESTAMP
   AND rpv.request_time_begin < $3::TIMESTAMP
   AND rpv.request_monitoring IS FALSE
   AND rpv.property_name_id = $4::INTEGER
  JOIN result_code rc ON rc.id = r.result_code_id
   AND rc.name NOT IN ('CommandFailed', 'CommandFailedServerClosingConnection')
  JOIN request_type rt ON rt.id = r.request_type_id
   AND rt.name NOT IN ('PollAcknowledgement', 'PollResponse')
 WHERE r.service_id = $1::INTEGER
   AND r.time_begin >= $2::TIMESTAMP
   AND r.time_begin < $3::TIMESTAMP
   AND r.is_monitoring IS FALSE
 GROUP BY r.user_name
 ORDER BY r.user_name;

