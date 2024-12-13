$datestamp = get-date -format "dd-MM-yyyy_HHmmss"
$git_hash = git rev-parse --short HEAD

$pathtracer = "./out/src/Release/pt.exe"
$config   = $args[0]
$samples  = if ($args.Count -ge 2) { $args[1] } else { 8 }
$bounces  = if ($args.Count -ge 3) { $args[2] } else { 3 }
$batch    = if ($args.Count -ge 4) { $args[3] } else { 10 }

$result_path = "artefacts/render_${git_hash}_${samples}_${bounces}_${datestamp}.png"

& $pathtracer $config $result_path $samples $bounces $batch
