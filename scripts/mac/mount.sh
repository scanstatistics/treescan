#!

# Mount nfse.omni.imsweb.com/prj/treescan onto Mac Mini

echo "mounting treescan -- use windows password"
mount -t smbfs //OMNI\;hostovic@nfse.omni.imsweb.com/treescan /Users/hostovic/prj/treescan.development/treescan.home
