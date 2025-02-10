npx esbuild main.ts --bundle --platform=browser --outfile=salam.min.js --external:fs --external:path
@REM npx terser salam.min.js --compress --mangle -o salam.min.js
@REM npx uglifyjs salam.min.js --compress --mangle --mangle-props --output salam.min.js
@REM npx javascript-obfuscator salam.min.js --output salam.min.js --compact true --control-flow-flattening true
@REM npx --yes javascript-obfuscator salam.min.js --output salam.min.js --compact true --control-flow-flattening true
