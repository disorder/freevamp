# Stolen from the copying.awk script included in gdb.

BEGIN {
  FS="\"";
  print "/* Do not modify this file!  It is created automatically by";
  print "   copying.awk.  Modify copying.awk instead. */";
  print "";
  print "char szCopying[] = ";
}

/^[ \t]*END OF TERMS AND CONDITIONS[ \t]*$/ {
  print ";";
  exit;
}

/^[ \t]*NO WARRANTY[ \t]*$/ {
  print ", szWarranty[] = ";
}

{
  if ($0 ~ /\f/) {
    print "  \"\\n\"";
  } else {
    printf "  \"";
    for (i = 1; i < NF; i++)
      printf "%s\\\"", $i;
    printf "%s\\n\"\n", $NF;
  }
}
