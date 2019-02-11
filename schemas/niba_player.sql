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
-- Name: player_character; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.player_character (
    name text NOT NULL,
    id text NOT NULL,
    gender "char" NOT NULL,
    strength integer DEFAULT 0,
    dexterity integer DEFAULT 0,
    physique integer DEFAULT 0,
    spirit integer DEFAULT 0
);


ALTER TABLE public.player_character OWNER TO postgres;

--
-- Name: player_magic; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.player_magic (
    magic_id integer NOT NULL,
    static_id integer NOT NULL,
    player_name text NOT NULL,
    priority integer NOT NULL,
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


ALTER TABLE public.player_magic OWNER TO postgres;

--
-- Name: player_magic_magic_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.player_magic_magic_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.player_magic_magic_id_seq OWNER TO postgres;

--
-- Name: player_magic_magic_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.player_magic_magic_id_seq OWNED BY public.player_magic.magic_id;


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
-- Name: player_magic magic_id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.player_magic ALTER COLUMN magic_id SET DEFAULT nextval('public.player_magic_magic_id_seq'::regclass);


--
-- Name: player_character player_character_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.player_character
    ADD CONSTRAINT player_character_pkey PRIMARY KEY (name);


--
-- Name: player_magic player_magic_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.player_magic
    ADD CONSTRAINT player_magic_pkey PRIMARY KEY (magic_id);


--
-- Name: user_id user_id_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.user_id
    ADD CONSTRAINT user_id_pkey PRIMARY KEY (id);


--
-- Name: player_character player_character_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.player_character
    ADD CONSTRAINT player_character_id_fkey FOREIGN KEY (id) REFERENCES public.user_id(id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- PostgreSQL database dump complete
--

