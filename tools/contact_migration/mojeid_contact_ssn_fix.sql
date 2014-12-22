COPY (
  SELECT
    c."registry_contact_id",
    c."username",
    c."birth_date",
    c."organization",
    c."vat_id_num",
    mu."validation_date"
  FROM "nicauth_contact" c
    JOIN "mojeid_mojeiduser" mu
      ON (mu."user_id" = c."id")
  WHERE c."is_active" = 't'
    AND mu."validation_date" IS NULL
    AND COALESCE(c."organization", '') = ''
    AND COALESCE(c."vat_id_num", '') != ''
    AND c."birth_date" IS NOT NULL
)
TO STDOUT
WITH CSV;
