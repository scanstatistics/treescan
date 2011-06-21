#!

# Mount nfsc.omni.imsweb.com/prj/satscan onto Mac Mini

echo "mounting satscan -- use windows password"
mount -t smbfs //OMNI\;hostovic@nfsc.omni.imsweb.com/satscan /Users/hostovic/prj/satscan
