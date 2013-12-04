
set(JOB_BUNDLE_SRC
  analysis.py
  auxiliary.py
  processor
)

add_custom_target(
  job_bundle ALL
  COMMAND tar "-C" "${CMAKE_CURRENT_SOURCE_DIR}" "-czf" "job_bundle.tar.gz" ${JOB_BUNDLE_SRC}
)
