npx esbuild main.ts --bundle --platform=browser --outfile=salam.min.js --external:fs --external:path
npx terser salam.min.js --compress --mangle -o salam.min.js
npx javascript-obfuscator salam.min.js --output salam.min.js --compact true --control-flow-flattening true
