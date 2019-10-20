
HEADINFO = []
#    OFFSET   SIZE    DESCRIPTION
HEADINFO_0= \
      0,      16,     'Header string: "SQLite format 3\000"'
HEADINFO.append(HEADINFO_0)

HEADINFO_1 = \
     16,       2,     'Page size in bytes.  (1 means 65536)'
HEADINFO.append(HEADINFO_1)

HEADINFO_2 = \
     18,       1,     'File format write version'
HEADINFO.append(HEADINFO_2)

HEADINFO_3 = \
     19,       1,     'File format read version'
HEADINFO.append(HEADINFO_3)

HEADINFO_4 = \
     20,       1,     'Bytes of unused space at the end of each page'
HEADINFO.append(HEADINFO_4)

HEADINFO_5 = \
     21,       1,     'Max embedded payload fraction (must be 64)'
HEADINFO.append(HEADINFO_5)

HEADINFO_6 = \
     22,       1,     'Min embedded payload fraction (must be 32)'
HEADINFO.append(HEADINFO_6)

HEADINFO_7 = \
     23,       1,     'Min leaf payload fraction (must be 32)'
HEADINFO.append(HEADINFO_7)

HEADINFO_8 = \
     24,       4,     'File change counter'
HEADINFO.append(HEADINFO_8)

HEADINFO_9 = \
     28,       4,     'Reserved for future use'
HEADINFO.append(HEADINFO_9)

HEADINFO_10 = \
     32,       4,     'First freelist page'
HEADINFO.append(HEADINFO_10)

HEADINFO_11 = \
     36,       4,     'Number of freelist pages in the file'
HEADINFO.append(HEADINFO_11)

HEADINFO_12 = \
     40,      60,     '15 4-byte meta values passed to higher layers'
HEADINFO.append(HEADINFO_12)

HEADINFO_13 = \
     40,       4,     'Schema cookie'
HEADINFO.append(HEADINFO_13)

HEADINFO_14 = \
     44,       4,     'File format of schema layer'
HEADINFO.append(HEADINFO_14)

HEADINFO_15 = \
     48,       4,     'Size of page cache'
HEADINFO.append(HEADINFO_15)

HEADINFO_16 = \
     52,       4,     'Largest root-page (auto/incr_vacuum)'
HEADINFO.append(HEADINFO_16)

HEADINFO_17 = \
     56,       4,     '1=UTF-8 2=UTF16le 3=UTF16be'
HEADINFO.append(HEADINFO_17)

HEADINFO_18 = \
     60,       4,     'User version'
HEADINFO.append(HEADINFO_18)

HEADINFO_19 = \
     64,       4,     'Incremental vacuum mode'
HEADINFO.append(HEADINFO_19)

HEADINFO_20 = \
     68,       4,     'Application-ID'
HEADINFO.append(HEADINFO_20)

HEADINFO_21 = \
     72,      20,     'unused'
HEADINFO.append(HEADINFO_21)

HEADINFO_22 = \
     92,       4,     'The version-valid-for number'
HEADINFO.append(HEADINFO_22)

HEADINFO_23 = \
     96,       4,     'SQLITE_VERSION_NUMBER'
HEADINFO.append(HEADINFO_23)


import sys

class DBHeader:
    bvalue = []
    def __init__(self, filename):
        self.dbfilename = filename
        rbdbfile = open(self.dbfilename, 'rb')
        for i in range(len(HEADINFO)):
            offset = HEADINFO[i][0]
            size = HEADINFO[i][1]
            rbdbfile.seek(offset)
            self.bvalue.append(rbdbfile.read(size))
        rbdbfile.close()

    def getHeaderString(self):
        result = HEADINFO_0[2] + ':\n\t' + str(self.bvalue[0])
        print(result)

        return result

    def getPageSize(self):
        result = HEADINFO_1[2] + ':\n\t' + str(self.bvalue[1])
        print(result)
        return result

    def dump(self):
        self.getHeaderString()
        self.getPageSize()


dbfile = None


def getHeaderString():
    offset = HEADINFO_0[0]
    size   = HEADINFO_0[1]
    description = HEADINFO_0[2]

    dbfile.seek(offset)
    value = dbfile.read(size)

    result = description + ':\n\t' + str(value)
    print(result)

    return result

    

def main(orig_args):
    global dbfile
    if not orig_args:
        print('Must pass a db file')
        sys.exit(1)

    dbfilename = orig_args[0]

    try:
        dbfile = open(dbfilename, mode='rb')
    except:
        print('Must pass a valid file')
        sys.exit(1)

    print(dbfilename)
    print(dbfile)

#    getHeaderString()
    dbheader = DBHeader(dbfilename)
    dbheader.dump()
    
    for i in range(len(HEADINFO)):
        print()
        #print(HEADINFO[i][2] + ':')
        #print('\t' + 'xxx')
        #print('-------------------------------')



if __name__ == '__main__':
    main(sys.argv[1:])
