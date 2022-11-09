cmake_minimum_required ( VERSION 3.0)

project(vinvout)

set(IMR_PATH "../../imr/1.0-r0/drivers/imr")
set(OSAL_PATH "= ../../osal/1.0-r0/os")

include_directories(${CMAKE_CURRENT_LIST_DIR}/vin)
include_directories(${CMAKE_CURRENT_LIST_DIR}/imr)
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/vin/include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/vout/include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/imr/include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/${OSAL_PATH}/osal/include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/${IMR_PATH}/include)

#link_directories(${CMAKE_CURRENT_LIST_DIR}/lib)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/vin input_src)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/vout output_src)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/imr imr_src)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src test_src)

add_executable(${PROJECT_NAME} ${test_src} ${input_src} ${output_src} ${imr_src})

target_link_libraries(${PROJECT_NAME} 
  osal_wrapper
  memory_allocator
  imr_v3h2
  imrdlg_v3h2
  m
  pthread
  atmlib_v3h2
  rivp_lde_v3h2
  drm
)

