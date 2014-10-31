-- contact_id,handle,type,street1,street2,street3,city,state,postal_code,country,company_name
COPY (
    SELECT nc.registry_contact_id,UPPER(nc.username),nat.code,na.street1,
           na.street2,na.street3,na.city,na.state,na.postal_code,na.country_id,
           na.company_name
    FROM nicauth_address na
    JOIN nicauth_addresstype nat ON (nat.id=na.type_id AND
                                     nat.code IN ('BILLING','SHIPPING','MAILING'))
    JOIN nicauth_contact nc ON (nc.id=na.contact_id AND nc.is_active)
    ORDER BY UPPER(nc.username),nat.id
) TO STDOUT WITH csv;
