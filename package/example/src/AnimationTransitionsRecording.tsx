import * as React from 'react'
import { useCallback } from 'react'
import { Button, Dimensions, PixelRatio, StyleSheet, View } from 'react-native'
import {
  Float3,
  useModel,
  useAssetAnimator,
  getAssetFromModel,
  FilamentProvider,
  useFilamentContext,
  useDisposableResource,
  useSkybox,
  useRecorder,
  useWorkletCallback,
} from 'react-native-filament'
import { useDefaultLight } from './hooks/useDefaultLight'
import { getAssetPath } from './utils/getAssetPasth'
import Video from 'react-native-video'

const penguModelPath = getAssetPath('pengu.glb')
const pirateHatPath = getAssetPath('pirate.glb')

// Camera config:
const cameraPosition: Float3 = [0, 0, 8]
const cameraTarget: Float3 = [0, 0, 0]
const cameraUp: Float3 = [0, 1, 0]
const focalLengthInMillimeters = 28
const near = 0.1
const far = 1000

const FPS = 60
const DURATION = 3 // seconds

function Renderer() {
  const { camera } = useFilamentContext()
  useDefaultLight()
  useSkybox({ color: '#88defb' })

  const pengu = useModel({ path: penguModelPath })
  const penguAsset = getAssetFromModel(pengu)
  const pirateHat = useModel({ path: pirateHatPath })
  const pirateHatAsset = getAssetFromModel(pirateHat)
  const pirateHatAnimator = useDisposableResource(() => {
    if (pirateHatAsset == null || penguAsset == null) {
      return undefined
    }
    return Promise.resolve(pirateHatAsset.createAnimatorWithAnimationsFrom(penguAsset))
  }, [pirateHatAsset, penguAsset])

  const penguAnimator = useAssetAnimator(penguAsset)

  const renderCallback = useCallback(
    (passedSeconds: number) => {
      'worklet'
      camera.lookAt(cameraPosition, cameraTarget, cameraUp)

      if (pirateHatAnimator == null || penguAnimator == null) {
        return
      }

      // Update the animators to play the current animation
      penguAnimator.applyAnimation(0, passedSeconds)
      pirateHatAnimator.applyAnimation(0, passedSeconds)

      penguAnimator.updateBoneMatrices()
      pirateHatAnimator.updateBoneMatrices()
    },
    [camera, pirateHatAnimator, penguAnimator]
  )

  const [videoUri, setVideoUri] = React.useState<string>()
  const recorder = useRecorder({
    bitRate: 2_000_000,
    fps: FPS,
    height: PixelRatio.getPixelSizeForLayoutSize(Dimensions.get('screen').height),
    width: PixelRatio.getPixelSizeForLayoutSize(Dimensions.get('screen').width),
  })
  const { engine } = useFilamentContext()
  const startRecording = useWorkletCallback(() => {
    'worklet'

    console.log('Starting rendering')
    const framesToRender = DURATION * FPS
    const started = Date.now()
    for (let i = 0; i < framesToRender; i++) {
      const nextTimestamp = started + i * (1 / FPS)
      console.log(`Rendering frame #${i + 1} of ${framesToRender} at ${nextTimestamp}`)
      const passedSeconds = nextTimestamp - started
      // Update the scene:
      renderCallback(passedSeconds)
      // Create the commands for the GPU:
      engine.render(nextTimestamp, false)
      // Wait for the GPU render to complete:
      engine.flushAndWait()
      // Render the current frame to the recorder:
      recorder.renderFrame(nextTimestamp)
    }
  }, [engine, recorder, renderCallback])

  // Configure camera
  React.useEffect(() => {
    const aspectRatio = recorder.width / recorder.height
    camera.setLensProjection(focalLengthInMillimeters, aspectRatio, near, far)
    console.log('Camera configured')
  }, [camera, recorder.height, recorder.width])

  return (
    <View style={styles.container}>
      {videoUri != null ? (
        <Video
          style={{ flex: 1 }}
          paused={false}
          repeat={true}
          controls={true}
          source={{ uri: videoUri }}
          onError={() => console.error(e)}
          onLoad={() => console.log('On load')}
          onEnd={() => console.log('On end')}
        />
      ) : null}
      <Button
        onPress={async () => {
          console.log('Starting recording...')
          await recorder.startRecording()
          await startRecording()
          // TODO: issue, we can't call stop when we don't know here if all frames have been processed yet
          console.log('Stopping recording')
          const uri = await recorder.stopRecording()
          console.log('Recording stopped.')
          console.log('Video URI:', uri)
          setVideoUri(uri)
        }}
        title={'Start recording'}
      />
    </View>
  )
}

export function AnimationTransitionsRecording() {
  return (
    <FilamentProvider>
      <Renderer />
    </FilamentProvider>
  )
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
  },
  filamentView: {
    flex: 1,
  },
  btnContainer: {
    height: 200,
    width: '100%',
    position: 'absolute',
    bottom: 0,
  },
})
