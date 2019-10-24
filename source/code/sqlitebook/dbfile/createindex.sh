#!/bin/sh

rm -rf myindex.db
sqlite3 myindex.db "CREATE TABLE info1 (name TEXT, age INT)"
sqlite3 myindex.db "INSERT INTO info1 VALUES ('ruby', 19)"
sqlite3 myindex.db "INSERT INTO info1 VALUES ('joe', 18)"
sqlite3 myindex.db "INSERT INTO info1 VALUES ('lily', 20)"
sqlite3 myindex.db "CREATE TABLE info2 (name TEXT, salary INT)"
sqlite3 myindex.db "INSERT INTO info2 VALUES ('joe', 16000)"
sqlite3 myindex.db "INSERT INTO info2 VALUES ('lily', 20000)"
sqlite3 myindex.db "INSERT INTO info1 VALUES ('jack', 18000)"
sqlite3 myindex.db "INSERT INTO info1 VALUES ('mam', 20000)"
