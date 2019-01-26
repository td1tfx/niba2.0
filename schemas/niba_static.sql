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
-- Name: 人物; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public."人物" (
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


ALTER TABLE public."人物" OWNER TO postgres;

--
-- Name: 人物武功; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public."人物武功" (
    id integer NOT NULL,
    fk_character_id integer,
    fk_magic_id integer,
    priority integer DEFAULT 0 NOT NULL
);


ALTER TABLE public."人物武功" OWNER TO postgres;

--
-- Name: COLUMN "人物武功".id; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN public."人物武功".id IS 'probably not needed, but is here so admin apps can modify stuff';


--
-- Name: COLUMN "人物武功".priority; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN public."人物武功".priority IS 'if we ever need to order them';


--
-- Name: 人物装备; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public."人物装备" (
    id integer NOT NULL,
    fk_character_id integer NOT NULL,
    fk_item_id integer NOT NULL,
    fk_item_type integer NOT NULL
);


ALTER TABLE public."人物装备" OWNER TO postgres;

--
-- Name: COLUMN "人物装备".id; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN public."人物装备".id IS '无用';


--
-- Name: COLUMN "人物装备".fk_item_type; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN public."人物装备".fk_item_type IS '装备类型';


--
-- Name: character_dump; Type: VIEW; Schema: public; Owner: postgres
--

CREATE VIEW public.character_dump AS
 SELECT to_json(array_agg(t.*)) AS to_json
   FROM ( SELECT "人物".character_id,
            "人物".name,
            row_to_json(( SELECT d.*::record AS d
                   FROM ( SELECT "人物".strength,
                            "人物".dexterity,
                            "人物".physique,
                            "人物".spirit) d)) AS attrs,
            row_to_json(( SELECT d.*::record AS d
                   FROM ( SELECT "人物".description,
                            "人物".hp,
                            "人物".mp,
                            "人物".attack_min,
                            "人物".attack_max,
                            "人物".inner_power,
                            "人物".accuracy,
                            "人物".evasion,
                            "人物".speed,
                            "人物".defence,
                            "人物".crit_chance,
                            "人物".crit_damage,
                            "人物".reduce_def,
                            "人物".reduce_def_perc,
                            "人物".hp_regen,
                            "人物".mp_regen,
                            "人物".gold_res,
                            "人物".wood_res,
                            "人物".water_res,
                            "人物".fire_res,
                            "人物".earth_res,
                            "人物".hp_on_hit,
                            "人物".hp_steal,
                            "人物".mp_on_hit,
                            "人物".mp_steal) d)) AS stats,
            COALESCE(magics_t.active_magic, '[]'::json) AS active_magic,
            COALESCE(items_t.items, '[]'::json) AS equipments
           FROM ((public."人物"
             LEFT JOIN ( SELECT "人物武功".fk_character_id,
                    json_agg("人物武功".fk_magic_id) AS active_magic
                   FROM public."人物武功"
                  GROUP BY "人物武功".fk_character_id) magics_t ON (("人物".character_id = magics_t.fk_character_id)))
             LEFT JOIN ( SELECT "人物装备".fk_character_id,
                    json_agg("人物装备".fk_item_id) AS items
                   FROM public."人物装备"
                  GROUP BY "人物装备".fk_character_id) items_t ON (("人物".character_id = items_t.fk_character_id)))
          ORDER BY "人物".character_id) t;


ALTER TABLE public.character_dump OWNER TO postgres;

--
-- Name: 装备; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public."装备" (
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


ALTER TABLE public."装备" OWNER TO postgres;

--
-- Name: COLUMN "装备".type; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN public."装备".type IS '装备类型（武器，衣服等）';


--
-- Name: item_dump; Type: VIEW; Schema: public; Owner: postgres
--

CREATE VIEW public.item_dump AS
 SELECT to_json(array_agg(t.*)) AS to_json
   FROM ( SELECT "装备".equipment_id,
            "装备".name,
            "装备".type,
            "装备".item_level,
            "装备".required_level,
            row_to_json(( SELECT d.*::record AS d
                   FROM ( SELECT "装备".description,
                            "装备".hp,
                            "装备".mp,
                            "装备".attack_min,
                            "装备".attack_max,
                            "装备".inner_power,
                            "装备".accuracy,
                            "装备".evasion,
                            "装备".speed,
                            "装备".defence,
                            "装备".crit_chance,
                            "装备".crit_damage,
                            "装备".reduce_def,
                            "装备".reduce_def_perc,
                            "装备".hp_regen,
                            "装备".mp_regen,
                            "装备".gold_res,
                            "装备".wood_res,
                            "装备".water_res,
                            "装备".fire_res,
                            "装备".earth_res,
                            "装备".hp_on_hit,
                            "装备".hp_steal,
                            "装备".mp_on_hit,
                            "装备".mp_steal) d)) AS stats
           FROM public."装备"
          ORDER BY "装备".type, "装备".equipment_id) t;


ALTER TABLE public.item_dump OWNER TO postgres;

--
-- Name: 武功; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public."武功" (
    magic_id integer NOT NULL,
    name text,
    active boolean DEFAULT true,
    multiplier integer DEFAULT 100,
    inner_damage integer DEFAULT 0,
    cd integer DEFAULT 0,
    mp_cost integer DEFAULT 0,
    inner_property integer DEFAULT 0 NOT NULL,
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


ALTER TABLE public."武功" OWNER TO postgres;

--
-- Name: magic_dump; Type: VIEW; Schema: public; Owner: postgres
--

CREATE VIEW public.magic_dump AS
 SELECT to_json(array_agg(t.*)) AS to_json
   FROM ( SELECT "武功".magic_id,
            "武功".name,
            "武功".active,
            "武功".multiplier,
            "武功".inner_damage,
            "武功".cd,
            "武功".mp_cost,
            "武功".inner_property,
            row_to_json(( SELECT d.*::record AS d
                   FROM ( SELECT "武功".description,
                            "武功".hp,
                            "武功".mp,
                            "武功".attack_min,
                            "武功".attack_max,
                            "武功".inner_power,
                            "武功".accuracy,
                            "武功".evasion,
                            "武功".speed,
                            "武功".defence,
                            "武功".crit_chance,
                            "武功".crit_damage,
                            "武功".reduce_def,
                            "武功".reduce_def_perc,
                            "武功".hp_regen,
                            "武功".mp_regen,
                            "武功".gold_res,
                            "武功".wood_res,
                            "武功".water_res,
                            "武功".fire_res,
                            "武功".earth_res,
                            "武功".hp_on_hit,
                            "武功".hp_steal,
                            "武功".mp_on_hit,
                            "武功".mp_steal) d)) AS stats
           FROM public."武功"
          ORDER BY "武功".magic_id) t;


ALTER TABLE public.magic_dump OWNER TO postgres;

--
-- Name: 人物_character_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public."人物_character_id_seq"
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public."人物_character_id_seq" OWNER TO postgres;

--
-- Name: 人物_character_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public."人物_character_id_seq" OWNED BY public."人物".character_id;


--
-- Name: 人物武功_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public."人物武功_id_seq"
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public."人物武功_id_seq" OWNER TO postgres;

--
-- Name: 人物武功_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public."人物武功_id_seq" OWNED BY public."人物武功".id;


--
-- Name: 人物装备_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public."人物装备_id_seq"
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public."人物装备_id_seq" OWNER TO postgres;

--
-- Name: 人物装备_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public."人物装备_id_seq" OWNED BY public."人物装备".id;


--
-- Name: 武功_inner_property_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public."武功_inner_property_seq"
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public."武功_inner_property_seq" OWNER TO postgres;

--
-- Name: 武功_inner_property_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public."武功_inner_property_seq" OWNED BY public."武功".inner_property;


--
-- Name: 武功_magic_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public."武功_magic_id_seq"
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public."武功_magic_id_seq" OWNER TO postgres;

--
-- Name: 武功_magic_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public."武功_magic_id_seq" OWNED BY public."武功".magic_id;


--
-- Name: 装备_item_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public."装备_item_id_seq"
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public."装备_item_id_seq" OWNER TO postgres;

--
-- Name: 装备_item_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public."装备_item_id_seq" OWNED BY public."装备".equipment_id;


--
-- Name: 装备类型; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public."装备类型" (
    type integer NOT NULL,
    description text NOT NULL
);


ALTER TABLE public."装备类型" OWNER TO postgres;

--
-- Name: TABLE "装备类型"; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public."装备类型" IS '幼儿园专用';


--
-- Name: 装备类型_type_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public."装备类型_type_seq"
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public."装备类型_type_seq" OWNER TO postgres;

--
-- Name: 装备类型_type_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public."装备类型_type_seq" OWNED BY public."装备类型".type;


--
-- Name: 人物 character_id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public."人物" ALTER COLUMN character_id SET DEFAULT nextval('public."人物_character_id_seq"'::regclass);


--
-- Name: 人物武功 id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public."人物武功" ALTER COLUMN id SET DEFAULT nextval('public."人物武功_id_seq"'::regclass);


--
-- Name: 人物装备 id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public."人物装备" ALTER COLUMN id SET DEFAULT nextval('public."人物装备_id_seq"'::regclass);


--
-- Name: 武功 magic_id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public."武功" ALTER COLUMN magic_id SET DEFAULT nextval('public."武功_magic_id_seq"'::regclass);


--
-- Name: 装备 equipment_id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public."装备" ALTER COLUMN equipment_id SET DEFAULT nextval('public."装备_item_id_seq"'::regclass);


--
-- Name: 装备类型 type; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public."装备类型" ALTER COLUMN type SET DEFAULT nextval('public."装备类型_type_seq"'::regclass);


--
-- Name: 人物武功 no_same_magic_per_char; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public."人物武功"
    ADD CONSTRAINT no_same_magic_per_char UNIQUE (fk_character_id, fk_magic_id);


--
-- Name: 人物 人物_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public."人物"
    ADD CONSTRAINT "人物_pkey" PRIMARY KEY (character_id);


--
-- Name: 人物武功 人物武功_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public."人物武功"
    ADD CONSTRAINT "人物武功_pkey" PRIMARY KEY (id);


--
-- Name: 人物装备 人物装备_fk_character_id_fk_item_type_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public."人物装备"
    ADD CONSTRAINT "人物装备_fk_character_id_fk_item_type_key" UNIQUE (fk_character_id, fk_item_type);


--
-- Name: 人物装备 人物装备_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public."人物装备"
    ADD CONSTRAINT "人物装备_pkey" PRIMARY KEY (id);


--
-- Name: 武功 武功_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public."武功"
    ADD CONSTRAINT "武功_pkey" PRIMARY KEY (magic_id);


--
-- Name: 装备 装备_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public."装备"
    ADD CONSTRAINT "装备_pkey" PRIMARY KEY (equipment_id);


--
-- Name: 装备类型 装备类型_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public."装备类型"
    ADD CONSTRAINT "装备类型_pkey" PRIMARY KEY (type);


--
-- Name: 人物武功 人物武功_fk_character_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public."人物武功"
    ADD CONSTRAINT "人物武功_fk_character_id_fkey" FOREIGN KEY (fk_character_id) REFERENCES public."人物"(character_id) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- Name: 人物武功 人物武功_fk_magic_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public."人物武功"
    ADD CONSTRAINT "人物武功_fk_magic_id_fkey" FOREIGN KEY (fk_magic_id) REFERENCES public."武功"(magic_id) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- Name: 人物装备 人物装备_fk_character_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public."人物装备"
    ADD CONSTRAINT "人物装备_fk_character_id_fkey" FOREIGN KEY (fk_character_id) REFERENCES public."人物"(character_id) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- Name: 人物装备 人物装备_fk_item_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public."人物装备"
    ADD CONSTRAINT "人物装备_fk_item_id_fkey" FOREIGN KEY (fk_item_id) REFERENCES public."装备"(equipment_id) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- Name: 人物装备 人物装备_fk_item_type_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public."人物装备"
    ADD CONSTRAINT "人物装备_fk_item_type_fkey" FOREIGN KEY (fk_item_type) REFERENCES public."装备类型"(type) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- Name: 装备 装备_type_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public."装备"
    ADD CONSTRAINT "装备_type_fkey" FOREIGN KEY (type) REFERENCES public."装备类型"(type) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- PostgreSQL database dump complete
--

