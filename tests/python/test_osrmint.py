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



    """
    def test_notequal(self):
        p1 = Persona(paisDocumento=2, tipoDocumento=5,
            tipoPersona=7, numeroDocumento='36776855')
        p2 = Persona(paisDocumento=1, tipoDocumento=5,
            tipoPersona=7, numeroDocumento='36776855')
        self.assertEqual(p1!=p2, True)
        self.assertNotEqual(p1==p2, True)

    def test_fromDict(self):
        p = Persona()
        d = dict(personaDTO=self.pDictPC)
        p.fromDict(d)
        for key in d['personaDTO']:
            self.assertEqual(getattr(p, key), d['personaDTO'][key])

    def test_fromJSON(self):
        p = Persona()
        d = dict(personaDTO=self.pDictPC)
        j = json.dumps(d)
        p.fromJSON(j)
        for key in d['personaDTO']:
            self.assertEqual(getattr(p, key), d['personaDTO'][key])

    def test_fromXML(self):
        p = Persona()
        p.fromXML(self.xmlPC)
        d = p.asDict()
        for key in d['personaDTO']:
            self.assertEqual(getattr(p, key), d['personaDTO'][key])

    def test_asDict(self):
        p = Persona(
            paisDocumento=self.pDictPB['paisDocumento'],
            tipoDocumento=self.pDictPB['tipoDocumento'],
            tipoPersona=self.pDictPB['tipoPersona'],
            numeroDocumento=self.pDictPB['numeroDocumento'],
        )
        self.assertEqual(p.asDict(), dict(personaDTO=self.pDictPB))

    def test_asJSON(self):
        p = Persona()
        for key in self.pDictPC:
            setattr(p, key, self.pDictPC[key])
        d = dict(personaDTO=self.pDictPC)
        dj = json.loads(p.asJSON())
        #j = json.dumps(d, sort_keys=True)
        self.assertEqual(dj, d)

    def test_asXML(self):
        import xml.etree.ElementTree as ET
        root = ET.fromstring(self.xmlPC)
        t = root.tag
        d = dict()
        for child in root:
            d[child.tag] = child.text
        doc = {t: d}
        p = Persona()
        for key in self.pDictPC:
            setattr(p, key, self.pDictPC[key])
        #self.assertEqual(p.asXML(), self.xmlPC)

    def test_validarRUT(self):
        pass

    def test_validarCI(self):
        p = Persona()
        p.fromDict(dict(personaDTO=self.pDictPB))
        for ci in self.notvalid_CIS:
            p.numeroDocumento = ci
            self.assertEqual(p.validarNumeroDocumento(), False)
        for ci in self.valid_CIS:
            p.numeroDocumento = ci
            self.assertEqual(p.validarNumeroDocumento(), True)
        for ci in self.invalid_CIS:
            p.numeroDocumento = ci
            self.assertEqual(p.validarNumeroDocumento(), False)

    """

if __name__ == '__main__':
    unittest.main()
