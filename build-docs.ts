const typedoc: any = require('gulp-typedoc');
const gulp: any = require('gulp');

gulp.task("docs", function() {
    return gulp
        .src(["obs_node.d.ts"])
        .pipe(typedoc({
            // TypeScript options (see typescript docs) 
            module: "commonjs",
            target: "es6",
            includeDeclarations: true,
            excludeExternals: true,
            excludePrivate: true,
            out: "./docs",
            name: "obs-studio Node Bindings by Streamlabs",
            ignoreCompilerErrors: false,
        }))
    ;
});

let runSequence = require('run-sequence');
runSequence('docs');