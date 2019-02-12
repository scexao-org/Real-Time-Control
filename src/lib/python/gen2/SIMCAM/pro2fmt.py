#!/usr/bin/env python
#
# pro2def.py -- Create SIMCAM status stubs from a StatusAlias.pro file
#
#[ Eric Jeschke (eric@naoj.org) --
#  Last edit: Wed May 20 13:11:24 HST 2009
#]
#
"""This program can be used to generate SIMCAM-compatible status definition lines used to
send status to the Gen2 OCS.

Usage:
    $ ./pro2fmt.py VGWQ

    Ignore status alias VGWD.RUN0
    $ ./pro2fmt.py VGWD --ignore=VGWD.RUN0

"""
import sys, time
import string
import SOSS.status.pro2def as pro2def

magic_number = 137

def make_filter(table, ignorelist):

    lower_igl = map(string.lower, ignorelist)

    def fn(aliasdef):
        if not (aliasdef.table == table):
            return False

        if aliasdef.alias.lower() in lower_igl:
            return False

        return True

    return fn

def convtype(stattype):
    if stattype == 'I':
        return 'd'
    if stattype == 'F':
        return 'f'
    return 's'

def alias2key(alias, keep_table=False):
    parts = alias.lower().split('.')
    if keep_table:
        return '_'.join(parts)
    else:
        return '_'.join(parts[1:])


def gettable(deffile, table, ignorelist=[]):

    l = pro2def.read_dotPro(deffile)
    res = filter(make_filter(table, ignorelist), l.values())

    res.sort(lambda x, y: cmp(x.offset, y.offset))
    return res

def getfmtstr(table, aliasdeflst, varmap, precision=6, indent=8):

    s_indent = ' ' * indent

    print "%s# Autogenerated on %s by pro2fmt.py" % (s_indent,
                                                     time.ctime())
    print "%s# Aliases for table %s:" % (s_indent, table)
    print "%s# %5.5s %4.4s %1.1s %-50.50s" % (s_indent,
                                              "Off",
                                              "Len",
                                              "T",
                                              "Alias")
    offset = 0
    pad_index = 0
    fmt_str = ""
    varlist = []
    for b in aliasdeflst:
        if b.offset < magic_number:
            print "%s# Skipping invalid alias: %s" % (s_indent, 
                                                      b.alias)
            continue
        alias_offset = b.offset - magic_number
        if (alias_offset <= offset) and (offset != 0):
            print "%s# Overlapping alias offset: %d (%s)" % (
                s_indent, alias_offset, b.alias)
            continue
        if offset < alias_offset:
            pad_index += 1
            pad_length = alias_offset - offset
            print "%s# %5d %4d   padding" % (s_indent, 
                                             offset, pad_length)
            padding = ',' + (' ' * (pad_length-1))
            fmt_str += padding
            offset += pad_length

        fmttype = convtype(b.sttype)
        print "%s# %5d %4d %1.1s %-50.50s" % (s_indent,
                                              offset,
                                              b.length,
                                              fmttype,
                                              b.alias)
        
        key = alias2key(b.alias)
        # Substitute a variable if one was defined for this alias,
        # otherwise use a mangled form of the alias name
        try:
            key = varmap[key]
        except KeyError:
            pass

        if fmttype == 's':
            f = '%%(%s)%d.%d%s' % (key, b.length, b.length, fmttype)
        elif fmttype == 'f':
            prec = min(precision, b.length)
            f = '%%(%s)%d.%d%s' % (key, b.length, prec, fmttype)
        else:
            f = '%%(%s)%d%s' % (key, b.length, fmttype)
        
        fmt_str += f
        varlist.append(key)
        offset += b.length

    print ""
    print "%s# Format string:" % (s_indent)
    print ('%sfmt_%s = "' % (s_indent, table)) + fmt_str + '"'

    print ""
    print "%s# Register our table:" % (s_indent)
    print ("%sself.tbl_%s = self.ocs.addStatusTable('%s', %s, fmt_%s)" % (
            s_indent, table, table, repr(varlist), table))


def readmap(mapfile):

    res = {}
    with open(mapfile, 'r') as in_f:
        for line in in_f:
            line = line.strip()
            if line.startswith('#') or len(line) == 0:
                continue

            alias, var = line.split(':')
            key = alias2key(alias.strip())
            res[key] = var.strip()

    return res


def main(options, args):

    table = args[0]

    varmap = {}
    if options.varmap:
        varmap = readmap(options.varmap)

    aliasdeflst = gettable(options.deffile, table,
                        ignorelist=options.ignorelist.split(','))
    getfmtstr(table, aliasdeflst, varmap,
              precision=options.precision,
              indent=options.indent)




if __name__ == '__main__':

    # Parse command line options with nifty new optparse module
    from optparse import OptionParser

    usage = "usage: %prog [options] tablename"
    optprs = OptionParser(usage=usage, version=('%%prog'))
    
    optprs.add_option("--debug", dest="debug", default=False, action="store_true",
                      help="Enter the pdb debugger on main()")
    optprs.add_option("--precision", dest="precision", metavar="NUM",
                      type="int", default=4,
                      help="Set default precision for floating point numbers")
    optprs.add_option("--profile", dest="profile", action="store_true",
                      default=False,
                      help="Run the profiler on main()")
    optprs.add_option("--deffile", dest="deffile", metavar="FILE",
                      default="../SOSS/status/StatusAlias.pro",
                      help="Read FILE for obtaining the status alias definitions")
    optprs.add_option("--ignore", dest="ignorelist",
                      default='',
                      help="Comma separated list of aliases to ignore")
    optprs.add_option("--indent", dest="indent", metavar="NUM",
                      type="int", default=8,
                      help="Set indentation for output")
    optprs.add_option("--varmap", dest="varmap", 
                      help="Provide an alias to variable mapping")

    (options, args) = optprs.parse_args(sys.argv[1:])

    if len(args) != 1:
        optprs.error("incorrect number of arguments")

    # Are we debugging this?
    if options.debug:
        import pdb

        pdb.run('main(options, args)')

    # Are we profiling this?
    elif options.profile:
        import profile

        print "%s profile:" % sys.argv[0]
        profile.run('main(options, args)')

    else:
        main(options, args)


# END

