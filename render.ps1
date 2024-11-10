$datestamp = get-date -format "dd-MM-yyyy_HHmmss"
$git_hash = git rev-parse --short HEAD

$pathtracer = "./build/Release/pt.exe"
$config = $args[0]
$samples = $args[1]
$bounces = 3
$batch = 10

$result_path = "artefacts/render_${git_hash}_${samples}_${bounces}_${datestamp}.png"

& $pathtracer $config $result_path $samples $bounces $batch
