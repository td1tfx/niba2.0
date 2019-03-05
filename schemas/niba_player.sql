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
-- Name: battlestats; Type: TYPE; Schema: public; Owner: postgres
--

CREATE TYPE public.battlestats AS (
	hp integer,
	mp integer,
	attack_min integer,
	attack_max integer,
	inner_power integer,
	accuracy integer,
	evasion integer,
	speed integer,
	defence integer,
	crit_chance integer,
	crit_damage integer,
	reduce_def integer,
	reduce_def_perc integer,
	hp_regen integer,
	mp_regen integer,
	gold_res integer,
	wood_res integer,
	water_res integer,
	fire_res integer,
	earth_res integer,
	hp_on_hit integer,
	hp_steal integer,
	mp_on_hit integer,
	mp_steal integer
);


ALTER TYPE public.battlestats OWNER TO postgres;

--
-- Name: character_four_attributes; Type: TYPE; Schema: public; Owner: postgres
--

CREATE TYPE public.character_four_attributes AS (
	strength integer,
	dexterity integer,
	physique integer,
	spirit integer
);


ALTER TYPE public.character_four_attributes OWNER TO postgres;

--
-- Name: TYPE character_four_attributes; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TYPE public.character_four_attributes IS 'strength, dexterity, physique, spirit';


--
-- Name: character_info; Type: TYPE; Schema: public; Owner: postgres
--

CREATE TYPE public.character_info AS (
	name text,
	gender "char",
	attrs public.character_four_attributes
);


ALTER TYPE public.character_info OWNER TO postgres;

--
-- Name: TYPE character_info; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TYPE public.character_info IS 'including name, gender and character_four_attributess';


--
-- Name: magic_info; Type: TYPE; Schema: public; Owner: postgres
--

CREATE TYPE public.magic_info AS (
	magic_id integer,
	name text,
	active integer,
	multiplier integer,
	inner_damage integer,
	cd integer,
	mp_cost integer,
	inner_property integer,
	description text,
	stats public.battlestats
);


ALTER TYPE public.magic_info OWNER TO postgres;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: character_equipped_magic; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.character_equipped_magic (
    character_name text NOT NULL,
    magics integer[]
);


ALTER TABLE public.character_equipped_magic OWNER TO postgres;

--
-- Name: player_character; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.player_character (
    id text NOT NULL,
    "character" public.character_info NOT NULL
);


ALTER TABLE public.player_character OWNER TO postgres;

--
-- Name: TABLE player_character; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON TABLE public.player_character IS 'stores user id -> character mapping';


--
-- Name: player_magic; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.player_magic (
    character_name text NOT NULL,
    magic_id integer NOT NULL,
    magic public.magic_info NOT NULL
);


ALTER TABLE public.player_magic OWNER TO postgres;

--
-- Name: COLUMN player_magic.magic; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN public.player_magic.magic IS 'note that magic itself contains the same magic_id; not a very good practice';


--
-- Name: user_id; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.user_id (
    id text NOT NULL,
    hashed_password bytea NOT NULL,
    salt bytea NOT NULL,
    logged_in boolean DEFAULT false
);


ALTER TABLE public.user_id OWNER TO postgres;

--
-- Name: character_equipped_magic character_equipped_magic_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.character_equipped_magic
    ADD CONSTRAINT character_equipped_magic_pkey PRIMARY KEY (character_name);


--
-- Name: player_character player_character_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.player_character
    ADD CONSTRAINT player_character_pkey PRIMARY KEY (id);


--
-- Name: player_magic player_magic_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.player_magic
    ADD CONSTRAINT player_magic_pkey PRIMARY KEY (character_name, magic_id);


--
-- Name: user_id user_id_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.user_id
    ADD CONSTRAINT user_id_pkey PRIMARY KEY (id);


--
-- PostgreSQL database dump complete
--
