# distutils: language = c++
# distutils: extra_compile_args = -std=c++11
# distutils: include_dirs = ../../include
# distutils: library_dirs = ../../src
# distutils: libraries = opendht gnutls
#
# opendht.pyx - Copyright 2015 by Guillaume Roguez <yomgui1 AT gmail DOT com>
# A Python3 wrapper to access to OpenDHT API
# This wrapper is written for Cython 0.22
# 
# This file is part of OpenDHT Python Wrapper.
#
#    OpenDHT Python Wrapper is free software:  you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    OpenDHT Python Wrapper is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with OpenDHT Python Wrapper. If not, see <http://www.gnu.org/licenses/>.
#


from libc.stdint cimport *
from libcpp.string cimport string
from libcpp.pair cimport pair
from cython.operator cimport dereference as deref

ctypedef uint16_t in_port_t
ctypedef unsigned short int sa_family_t;


cdef extern from "<memory>" namespace "std" nogil:
    cdef cppclass shared_ptr[T]:
        shared_ptr() except +
        T* get()
        T operator*()


cdef extern from "opendht/infohash.h" namespace "dht":
    cdef cppclass InfoHash:
        InfoHash() except +
        string toString()


cdef extern from "opendht/crypto.h" namespace "dht::crypto":
    ctypedef pair[shared_ptr[PrivateKey], shared_ptr[Certificate]] Identity
    cdef Identity generateIdentity()

    cdef cppclass PrivateKey:
        PrivateKey()
        PublicKey getPublicKey() const

    cdef cppclass PublicKey:
        PublicKey()
        InfoHash getId() const

    cdef cppclass Certificate:
        Certificate()
        InfoHash getId() const


cdef class _WithID:
    def __repr__(self):
        return "<%s '%s'>" % (self.__class__.__name__, str(self))
    def __str__(self):
        return self.getId().decode()


cdef class PyPublicKey(_WithID):
    cdef PublicKey _key
    def getId(self):
        return self._key.getId().toString()


cdef class PySharedCertificate(_WithID):
    cdef shared_ptr[Certificate] _cert
    def getId(self):
        return self._cert.get().getId().toString()


cdef class PyIdentity:
    cdef Identity _id;
    def generate(self):
        self._id = generateIdentity()
    property PublicKey:
        def __get__(self):
            k = PyPublicKey()
            k._key = self._id.first.get().getPublicKey()
            return k
    property Certificate:
        def __get__(self):
            c = PySharedCertificate()
            c._cert = self._id.second
            return c


cdef extern from "opendht/dhtrunner.h" namespace "dht":
    cdef cppclass DhtRunner:
        DhtRunner() except +
        InfoHash getId() const
        InfoHash getRoutingId() const
        void bootstrap(const char*, const char*)
        void run(in_port_t, const Identity, bint)
        void join()
        bint isRunning()
        string getStorageLog() const
        string getRoutingTablesLog(sa_family_t af) const
        string getSearchesLog(sa_family_t af) const


cdef class PyDhtRunner(_WithID):
    cdef DhtRunner* thisptr;
    def __cinit__(self):
        self.thisptr = new DhtRunner()
    def getId(self):
        return self.thisptr.getId().toString()
    def getRoutingId(self):
        return self.thisptr.getRoutingId().toString()
    def bootstrap(self, bytes host, bytes port):
        self.thisptr.bootstrap(host, port)
    def run(self, in_port_t port, PyIdentity id, bint threaded=False):
        self.thisptr.run(port, id._id, threaded)
    def join(self):
        self.thisptr.join()
    def isRunning(self):
        return self.thisptr.isRunning()
    def getStorageLog(self):
        return self.thisptr.getStorageLog()
    def getRoutingTablesLog(self, sa_family_t af):
        return self.thisptr.getRoutingTablesLog(af)
    def getSearchesLog(self, sa_family_t af):
        return self.thisptr.getSearchesLog(af)
