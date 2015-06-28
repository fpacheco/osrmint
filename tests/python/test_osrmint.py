# -*- coding: utf-8 -*-
import unittest
from datetime import date
import json

import sys
import os
from pydal import DAL

class TestOsrmint(unittest.TestCase):

    def setUp(self):
        self.url = "http://127.0.0.1:5000/viaroute"
        self.nr = [10,100,1000]
        self.db = DAL('postgres://postgres:12rfpv12@localhost/osrminttest')

    def uniqueTableName(self, pre='A', jchar='_'):
        import uuid
        return "%s%s" % ( pre, jchar.join(str(i) for i in uuid.uuid4().fields) )

    def test_routeOkFromNumbers(self):
        db = self.db

        id = 1234
        sql = """SELECT
id, tdist, ttime
FROM osrmint_route(
'SELECT CAST(%d AS integer) AS id, CAST(-56.0961 AS float) AS xf, CAST(-34.8496 AS float) AS yf, CAST(-56.1296 AS float) AS xt, CAST(-34.8993 AS float) AS yt',
'http://127.0.0.1:5000/viaroute'
);""" % (id)
        rows = db.executesql(sql)
        self.assertEqual(len(rows)==1, True)
        self.assertEqual(rows[0][0]==id, True)
        self.assertEqual(rows[0][1]>0, True)
        self.assertEqual(rows[0][2]>0, True)

    def test_routeOkFromNumbersSave(self):
        db = self.db

        id = 1234
        temptable = self.uniqueTableName()
        sql = """SELECT
    id, tdist, ttime INTO TEMP %s
FROM
    osrmint_route(
    'SELECT CAST(%d AS integer) AS id, CAST(-56.0961 AS float) AS xf, CAST(-34.8496 AS float) AS yf, CAST(-56.1296 AS float) AS xt, CAST(-34.8993 AS float) AS yt',
    'http://127.0.0.1:5000/viaroute'
    );""" % (temptable,id)
        rows = db.executesql(sql)

        sql = "SELECT * FROM %s" % (temptable)
        rows = db.executesql(sql)
        self.assertEqual(len(rows)==1, True)
        self.assertEqual(rows[0][0]==id, True)
        self.assertEqual(rows[0][1]>0, True)
        self.assertEqual(rows[0][2]>0, True)

    def test_routeOkFromTables(self):
        db = self.db

        for nr in self.nr:
            sql = """SELECT
    id, tdist, ttime
FROM
    osrmint_route(
        'SELECT * FROM route ORDER BY random() LIMIT %d',
        'http://127.0.0.1:5000/viaroute'
    );""" % (nr)
            rows = db.executesql(sql)
            self.assertEqual(len(rows)==nr, True)
            self.assertEqual(rows[0][0]>0, True)
            self.assertEqual(rows[0][1]>0, True)
            self.assertEqual(rows[0][2]>0, True)

    def test_routeOkFromTablesSave(self):
        db = self.db

        for nr in self.nr:
            temptable = self.uniqueTableName()
            sql = """SELECT
    id, tdist, ttime INTO TEMP %s
FROM
    osrmint_route(
        'SELECT * FROM route ORDER BY random() LIMIT %d',
        'http://127.0.0.1:5000/viaroute'
    );""" % (temptable, nr)
            rows = db.executesql(sql)

            sql = "SELECT * FROM %s" % (temptable)
            rows = db.executesql(sql)
            self.assertEqual(len(rows)==nr, True)
            self.assertEqual(rows[0][0]>0, True)
            self.assertEqual(rows[0][1]>0, True)
            self.assertEqual(rows[0][2]>0, True)

    def test_routeFailFromTables(self):
        db = self.db

        sql = """SELECT id, tdist, ttime FROM osrmint_route('SELECT * FROM route WHERE id=-99999 ORDER BY random() LIMIT 50','http://127.0.0.1:5000/viaroute');"""
        # mejor usar self.assertRaises(roman.OutOfRangeError, roman.toRoman, 4000)
        try:
            rows = db.executesql(sql)
        except Exception, e:
            print "%s" % e
            self.assertEqual('%s' % e == 'La consulta no devuelve datos', True)

if __name__ == '__main__':
    unittest.main()
