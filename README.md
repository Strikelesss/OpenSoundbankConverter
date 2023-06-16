## OpenSoundbankConverter

An open source tool for converting and viewing soundbank files.

The current formats supported are:
* .E4B
* .SF2

The E4B format can also be read to extract internal sequences.

## Dependencies
 - sf2cute (creating SF2 files)
 - TinySoundFont (extracting SF2 data) 
   
NOTE: TinySoundFont has been modified to allow for specific reading needs, which is why it's been kept in this tool and not converted to a git submodule.
The hope is to eventually create an SF2 reader within this tool and remove the requirement of applying modifications to TSF which is strictly intended play SF2 files.
