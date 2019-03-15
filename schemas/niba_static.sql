--
-- PostgreSQL database dump
--

-- Dumped from database version 10.6 (Ubuntu 10.6-0ubuntu0.18.04.1)
-- Dumped by pg_dump version 10.6 (Ubuntu 10.6-0ubuntu0.18.04.1)

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET client_min_messages = warning;
SET row_security = off;

--
-- Name: plpgsql; Type: EXTENSION; Schema: -; Owner: 
--

CREATE EXTENSION IF NOT EXISTS plpgsql WITH SCHEMA pg_catalog;


--
-- Name: EXTENSION plpgsql; Type: COMMENT; Schema: -; Owner: 
--

COMMENT ON EXTENSION plpgsql IS 'PL/pgSQL procedural language';


SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: character_attribute; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.character_attribute (
    character_id integer NOT NULL,
    name text,
    strength integer DEFAULT 0,
    dexterity integer DEFAULT 0,
    physique integer DEFAULT 0,
    spirit integer DEFAULT 0,
    description text DEFAULT ''::text,
    hp integer DEFAULT 0,
    mp integer DEFAULT 0,
    attack_min integer DEFAULT 0,
    attack_max integer DEFAULT 0,
    inner_power integer DEFAULT 0,
    accuracy integer DEFAULT 0,
    evasion integer DEFAULT 0,
    speed integer DEFAULT 0,
    defence integer DEFAULT 0,
    crit_chance integer DEFAULT 0,
    crit_damage integer DEFAULT 0,
    reduce_def integer DEFAULT 0,
    reduce_def_perc integer DEFAULT 0,
    hp_regen integer DEFAULT 0,
    mp_regen integer DEFAULT 0,
    gold_res integer DEFAULT 0,
    wood_res integer DEFAULT 0,
    water_res integer DEFAULT 0,
    fire_res integer DEFAULT 0,
    earth_res integer DEFAULT 0,
    hp_on_hit integer DEFAULT 0,
    hp_steal integer DEFAULT 0,
    mp_on_hit integer DEFAULT 0,
    mp_steal integer DEFAULT 0
);


ALTER TABLE public.character_attribute OWNER TO postgres;

--
-- Name: character_attribute_character_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.character_attribute_character_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.character_attribute_character_id_seq OWNER TO postgres;

--
-- Name: character_attribute_character_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.character_attribute_character_id_seq OWNED BY public.character_attribute.character_id;


--
-- Name: character_equipment; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.character_equipment (
    id integer NOT NULL,
    fk_character_id integer NOT NULL,
    fk_item_id integer NOT NULL,
    fk_item_type integer NOT NULL
);


ALTER TABLE public.character_equipment OWNER TO postgres;

--
-- Name: COLUMN character_equipment.id; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN public.character_equipment.id IS '无用';


--
-- Name: COLUMN character_equipment.fk_item_type; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN public.character_equipment.fk_item_type IS '装备类型';


--
-- Name: character_magic; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.character_magic (
    id integer NOT NULL,
    fk_character_id integer,
    fk_magic_id integer,
    priority integer DEFAULT 0 NOT NULL
);


ALTER TABLE public.character_magic OWNER TO postgres;

--
-- Name: COLUMN character_magic.id; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN public.character_magic.id IS 'probably not needed, but is here so admin apps can modify stuff';


--
-- Name: COLUMN character_magic.priority; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN public.character_magic.priority IS 'if we ever need to order them';


--
-- Name: character_dump; Type: VIEW; Schema: public; Owner: postgres
--

CREATE VIEW public.character_dump AS
 SELECT to_json(array_agg(t.*)) AS to_json
   FROM ( SELECT character_attribute.character_id,
            character_attribute.name,
            character_attribute.description,
            row_to_json(( SELECT d.*::record AS d
                   FROM ( SELECT character_attribute.strength,
                            character_attribute.dexterity,
                            character_attribute.physique,
                            character_attribute.spirit) d)) AS attrs,
            row_to_json(( SELECT d.*::record AS d
                   FROM ( SELECT character_attribute.hp,
                            character_attribute.mp,
                            character_attribute.attack_min,
                            character_attribute.attack_max,
                            character_attribute.inner_power,
                            character_attribute.accuracy,
                            character_attribute.evasion,
                            character_attribute.speed,
                            character_attribute.defence,
                            character_attribute.crit_chance,
                            character_attribute.crit_damage,
                            character_attribute.reduce_def,
                            character_attribute.reduce_def_perc,
                            character_attribute.hp_regen,
                            character_attribute.mp_regen,
                            character_attribute.gold_res,
                            character_attribute.wood_res,
                            character_attribute.water_res,
                            character_attribute.fire_res,
                            character_attribute.earth_res,
                            character_attribute.hp_on_hit,
                            character_attribute.hp_steal,
                            character_attribute.mp_on_hit,
                            character_attribute.mp_steal) d)) AS stats,
            COALESCE(magics_t.active_magic, '[]'::json) AS active_magic,
            COALESCE(items_t.items, '[]'::json) AS equipments
           FROM ((public.character_attribute
             LEFT JOIN ( SELECT character_magic.fk_character_id,
                    json_agg(character_magic.fk_magic_id) AS active_magic
                   FROM public.character_magic
                  GROUP BY character_magic.fk_character_id) magics_t ON ((character_attribute.character_id = magics_t.fk_character_id)))
             LEFT JOIN ( SELECT character_equipment.fk_character_id,
                    json_agg(character_equipment.fk_item_id) AS items
                   FROM public.character_equipment
                  GROUP BY character_equipment.fk_character_id) items_t ON ((character_attribute.character_id = items_t.fk_character_id)))
          ORDER BY character_attribute.character_id) t;


ALTER TABLE public.character_dump OWNER TO postgres;

--
-- Name: character_equipment_item_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.character_equipment_item_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.character_equipment_item_id_seq OWNER TO postgres;

--
-- Name: character_equipment_item_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.character_equipment_item_id_seq OWNED BY public.character_equipment.id;


--
-- Name: character_magic_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.character_magic_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.character_magic_id_seq OWNER TO postgres;

--
-- Name: character_magic_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.character_magic_id_seq OWNED BY public.character_magic.id;


--
-- Name: equipment; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.equipment (
    equipment_id integer NOT NULL,
    name text NOT NULL,
    description text DEFAULT ''::text NOT NULL,
    type integer NOT NULL,
    item_level integer DEFAULT 0 NOT NULL,
    required_level integer DEFAULT 0 NOT NULL,
    hp integer DEFAULT 0,
    mp integer DEFAULT 0,
    attack_min integer DEFAULT 0,
    attack_max integer DEFAULT 0,
    inner_power integer DEFAULT 0,
    accuracy integer DEFAULT 0,
    evasion integer DEFAULT 0,
    speed integer DEFAULT 0,
    defence integer DEFAULT 0,
    crit_chance integer DEFAULT 0,
    crit_damage integer DEFAULT 0,
    reduce_def integer DEFAULT 0,
    reduce_def_perc integer DEFAULT 0,
    hp_regen integer DEFAULT 0,
    mp_regen integer DEFAULT 0,
    gold_res integer DEFAULT 0,
    wood_res integer DEFAULT 0,
    water_res integer DEFAULT 0,
    fire_res integer DEFAULT 0,
    earth_res integer DEFAULT 0,
    hp_on_hit integer DEFAULT 0,
    hp_steal integer DEFAULT 0,
    mp_on_hit integer DEFAULT 0,
    mp_steal integer DEFAULT 0
);


ALTER TABLE public.equipment OWNER TO postgres;

--
-- Name: COLUMN equipment.type; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN public.equipment.type IS '装备类型（武器，衣服等）';


--
-- Name: equipment_equipment_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.equipment_equipment_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.equipment_equipment_id_seq OWNER TO postgres;

--
-- Name: equipment_equipment_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.equipment_equipment_id_seq OWNED BY public.equipment.equipment_id;


--
-- Name: equipment_type; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.equipment_type (
    type integer NOT NULL,
    description text NOT NULL
);


ALTER TABLE public.equipment_type OWNER TO postgres;

--
-- Name: TABLE equipment_type; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.equipment_type IS '幼儿园专用';


--
-- Name: equipment_type_type_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.equipment_type_type_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.equipment_type_type_seq OWNER TO postgres;

--
-- Name: equipment_type_type_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.equipment_type_type_seq OWNED BY public.equipment_type.type;


--
-- Name: item_dump; Type: VIEW; Schema: public; Owner: postgres
--

CREATE VIEW public.item_dump AS
 SELECT to_json(array_agg(t.*)) AS to_json
   FROM ( SELECT equipment.equipment_id,
            equipment.name,
            equipment.description,
            equipment.type,
            equipment.item_level,
            equipment.required_level,
            row_to_json(( SELECT d.*::record AS d
                   FROM ( SELECT equipment.hp,
                            equipment.mp,
                            equipment.attack_min,
                            equipment.attack_max,
                            equipment.inner_power,
                            equipment.accuracy,
                            equipment.evasion,
                            equipment.speed,
                            equipment.defence,
                            equipment.crit_chance,
                            equipment.crit_damage,
                            equipment.reduce_def,
                            equipment.reduce_def_perc,
                            equipment.hp_regen,
                            equipment.mp_regen,
                            equipment.gold_res,
                            equipment.wood_res,
                            equipment.water_res,
                            equipment.fire_res,
                            equipment.earth_res,
                            equipment.hp_on_hit,
                            equipment.hp_steal,
                            equipment.mp_on_hit,
                            equipment.mp_steal) d)) AS stats
           FROM public.equipment
          ORDER BY equipment.type, equipment.equipment_id) t;


ALTER TABLE public.item_dump OWNER TO postgres;

--
-- Name: magic; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.magic (
    magic_id integer NOT NULL,
    name text,
    active boolean DEFAULT true,
    multiplier integer DEFAULT 100,
    inner_damage integer DEFAULT 0,
    cd integer DEFAULT 0,
    mp_cost integer DEFAULT 0,
    inner_property "char" DEFAULT 'j' NOT NULL,
    description text DEFAULT ''::text,
    hp integer DEFAULT 0,
    mp integer DEFAULT 0,
    attack_min integer DEFAULT 0,
    attack_max integer DEFAULT 0,
    inner_power integer DEFAULT 0,
    accuracy integer DEFAULT 0,
    evasion integer DEFAULT 0,
    speed integer DEFAULT 0,
    defence integer DEFAULT 0,
    crit_chance integer DEFAULT 0,
    crit_damage integer DEFAULT 0,
    reduce_def integer DEFAULT 0,
    reduce_def_perc integer DEFAULT 0,
    hp_regen integer DEFAULT 0,
    mp_regen integer DEFAULT 0,
    gold_res integer DEFAULT 0,
    wood_res integer DEFAULT 0,
    water_res integer DEFAULT 0,
    fire_res integer DEFAULT 0,
    earth_res integer DEFAULT 0,
    hp_on_hit integer DEFAULT 0,
    hp_steal integer DEFAULT 0,
    mp_on_hit integer DEFAULT 0,
    mp_steal integer DEFAULT 0
);


ALTER TABLE public.magic OWNER TO postgres;

--
-- Name: magic_dump; Type: VIEW; Schema: public; Owner: postgres
--

CREATE VIEW public.magic_dump AS
 SELECT to_json(array_agg(t.*)) AS to_json
   FROM ( SELECT magic.magic_id,
            magic.name,
            magic.description,
            magic.active,
            magic.multiplier,
            magic.inner_damage,
            magic.cd,
            magic.mp_cost,
            magic.inner_property,
            row_to_json(( SELECT d.*::record AS d
                   FROM ( SELECT magic.hp,
                            magic.mp,
                            magic.attack_min,
                            magic.attack_max,
                            magic.inner_power,
                            magic.accuracy,
                            magic.evasion,
                            magic.speed,
                            magic.defence,
                            magic.crit_chance,
                            magic.crit_damage,
                            magic.reduce_def,
                            magic.reduce_def_perc,
                            magic.hp_regen,
                            magic.mp_regen,
                            magic.gold_res,
                            magic.wood_res,
                            magic.water_res,
                            magic.fire_res,
                            magic.earth_res,
                            magic.hp_on_hit,
                            magic.hp_steal,
                            magic.mp_on_hit,
                            magic.mp_steal) d)) AS stats
           FROM public.magic
          ORDER BY magic.magic_id) t;


ALTER TABLE public.magic_dump OWNER TO postgres;

--
-- Name: magic_inner_property_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.magic_inner_property_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.magic_inner_property_seq OWNER TO postgres;

--
-- Name: magic_inner_property_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.magic_inner_property_seq OWNED BY public.magic.inner_property;


--
-- Name: magic_magic_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.magic_magic_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.magic_magic_id_seq OWNER TO postgres;

--
-- Name: magic_magic_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.magic_magic_id_seq OWNED BY public.magic.magic_id;


--
-- Name: character_attribute character_id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.character_attribute ALTER COLUMN character_id SET DEFAULT nextval('public.character_attribute_character_id_seq'::regclass);


--
-- Name: character_equipment id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.character_equipment ALTER COLUMN id SET DEFAULT nextval('public.character_equipment_item_id_seq'::regclass);


--
-- Name: character_magic id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.character_magic ALTER COLUMN id SET DEFAULT nextval('public.character_magic_id_seq'::regclass);


--
-- Name: equipment equipment_id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.equipment ALTER COLUMN equipment_id SET DEFAULT nextval('public.equipment_equipment_id_seq'::regclass);


--
-- Name: equipment_type type; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.equipment_type ALTER COLUMN type SET DEFAULT nextval('public.equipment_type_type_seq'::regclass);


--
-- Name: magic magic_id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.magic ALTER COLUMN magic_id SET DEFAULT nextval('public.magic_magic_id_seq'::regclass);


--
-- Name: character_equipment character_equipment_fk_character_id_fk_item_type_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.character_equipment
    ADD CONSTRAINT character_equipment_fk_character_id_fk_item_type_key UNIQUE (fk_character_id, fk_item_type);


--
-- Name: character_equipment character_equipment_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.character_equipment
    ADD CONSTRAINT character_equipment_pkey PRIMARY KEY (id);


--
-- Name: character_magic character_magic_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.character_magic
    ADD CONSTRAINT character_magic_pkey PRIMARY KEY (id);


--
-- Name: character_attribute character_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.character_attribute
    ADD CONSTRAINT character_pkey PRIMARY KEY (character_id);


--
-- Name: equipment equipment_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.equipment
    ADD CONSTRAINT equipment_pkey PRIMARY KEY (equipment_id);


--
-- Name: equipment_type equipment_type_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.equipment_type
    ADD CONSTRAINT equipment_type_pkey PRIMARY KEY (type);


--
-- Name: magic magic_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.magic
    ADD CONSTRAINT magic_pkey PRIMARY KEY (magic_id);


--
-- Name: character_magic no_same_magic_per_char; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.character_magic
    ADD CONSTRAINT no_same_magic_per_char UNIQUE (fk_character_id, fk_magic_id);


--
-- Name: character_equipment character_equipment_fk_character_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.character_equipment
    ADD CONSTRAINT character_equipment_fk_character_id_fkey FOREIGN KEY (fk_character_id) REFERENCES public.character_attribute(character_id) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- Name: character_equipment character_equipment_fk_item_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.character_equipment
    ADD CONSTRAINT character_equipment_fk_item_id_fkey FOREIGN KEY (fk_item_id) REFERENCES public.equipment(equipment_id) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- Name: character_equipment character_equipment_fk_item_type_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.character_equipment
    ADD CONSTRAINT character_equipment_fk_item_type_fkey FOREIGN KEY (fk_item_type) REFERENCES public.equipment_type(type) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- Name: character_magic character_magic_fk_character_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.character_magic
    ADD CONSTRAINT character_magic_fk_character_id_fkey FOREIGN KEY (fk_character_id) REFERENCES public.character_attribute(character_id) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- Name: character_magic character_magic_fk_magic_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.character_magic
    ADD CONSTRAINT character_magic_fk_magic_id_fkey FOREIGN KEY (fk_magic_id) REFERENCES public.magic(magic_id) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- Name: equipment equipment_type_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.equipment
    ADD CONSTRAINT equipment_type_fkey FOREIGN KEY (type) REFERENCES public.equipment_type(type) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- PostgreSQL database dump complete
--

