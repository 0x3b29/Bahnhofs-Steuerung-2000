# Bootstrap 

Bootstrap v5.2.3 (https://getbootstrap.com/)
Copyright 2011-2022 The Bootstrap Authors
Licensed under MIT (https://github.com/twbs/bootstrap/blob/main/LICENSE)

* The file ```./styles_uncompressed.h``` contains a subset of Bootstrap v5.2.3 
* The file ```./styles.h``` contains a subset of Bootstrap v5.2.3 in a minified and compressed version

To regenerate ```./styles.h```, run the script ```./generate_styles_h.js```

# Grape Nuts

Designed by Robert Leuschke 

Copyright 2003 The Grape Nuts Project Authors (https://github.com/googlefonts/grapenuts)

This Font Software is licensed under the SIL Open Font License, Version 1.1 . This license is copied below, and is also available with a FAQ at: https://openfontlicense.org

SIL OPEN FONT LICENSE Version 1.1 - 26 February 2007 


* The file ```./font_woff2.h``` contains a subset of the characters from the font https://github.com/googlefonts/grapenuts

To regenerate the font subset, the following command was used:

```
pyftsubset GrapeNuts-Regular.ttf \
  --text="BSabdefghilmnorstuvy02 ." \
  --unicodes="U+2665" \
  --output-file=grapenuts-subset.woff2 \
  --flavor=woff2
```