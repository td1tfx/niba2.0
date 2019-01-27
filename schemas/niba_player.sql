--
-- PostgreSQL database dump
--

-- Dumped from database version 11.1
-- Dumped by pg_dump version 11.1

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET client_min_messages = warning;
SET row_security = off;

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
-- Name: player_character player_character_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.player_character
    ADD CONSTRAINT player_character_pkey PRIMARY KEY (name);


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

