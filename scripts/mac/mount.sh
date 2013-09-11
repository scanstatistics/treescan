#!

# Mount nfsf.omni.imsweb.com/prj/treescan onto Mac Mini

echo "mounting treescan -- use windows password"
mount -t smbfs //OMNI\;hostovic@nfsf.omni.imsweb.com/treescan /Users/hostovic/prj/treescan.development/treescan.home
